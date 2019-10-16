#include "utils/cuda.hpp"
#include "sph_scene.hpp"

//#include <cublas.h>
#include <cuda_runtime.h>
#include <thrust/device_ptr.h>
#include <thrust/device_vector.h>

#include <thrust/sort.h>

using namespace px;

struct CudaSPHBuffer
{
    unsigned int capacity = 0;

    // because of the limit of max threads on a GPU
    // multiple streams cannot bring observable improvement
    static constexpr int n_streams = 0;
    cudaStream_t *streams = nullptr;

    float *mass = nullptr;
    float *rest_density = nullptr;
    float *coef_viscosity = nullptr;
    float3 *velocity = nullptr;         // velocity
    float3 *velocity_h = nullptr;       // half velocity
    float3 *acceleration = nullptr;     // acceleration
    float *density = nullptr;           // density
    float *pressure = nullptr;          // pressure
    int *grid_index = nullptr;          // index of the grid where the particle is
    int3 *cell_index = nullptr;         // x, y, z component of the grid index
    int *bucket = nullptr;         //
    int *bucket_start = nullptr;   // first index of the particles in a grid, ordered in the sorted index array
    int *bucket_map = nullptr;          // sorted index -> original index in the given velocity and position array
    int *grid_counter = nullptr;        // grid counter

    explicit CudaSPHBuffer() {}
} cuda_buffer;

__global__
void cudaSPHPosition2Grid(const float3 *__restrict__ pos,
                          int *__restrict__ grid_index, int3 *__restrict__ cell_index,
                          const float3 lower_boundary, const float smooth_len, const int3 resolution,
                          const unsigned int n, const int offset)
{
    PX_CUDA_LOOP(id, n)
    {
        auto i = id + offset;
        auto p = pos[i];
        auto x = static_cast<int>((p.x - lower_boundary.x) / smooth_len);
        if (x < 0) x = 0;
        else if (x > resolution.x-1) x = resolution.x-1;
        auto y = static_cast<int>((p.y - lower_boundary.y) / smooth_len);
        if (y < 0) y = 0;
        else if (y > resolution.y-1) y = resolution.y-1;
        auto z = static_cast<int>((p.z - lower_boundary.z) / smooth_len);
        if (z < 0) z = 0;
        else if (z > resolution.z-1) z = resolution.z-1;

        cell_index[i].x = x;
        cell_index[i].y = y;
        cell_index[i].z = z;
        grid_index[i] = (x*resolution.y + y)*resolution.z + z;
    }
}
__global__
void cudaSPHClearCounter(int *__restrict__ grid_counter, const unsigned int n_grids)
{
    PX_CUDA_LOOP(i, n_grids)
    {
        grid_counter[i] = 0;
    }
}
__global__
void cudaSPHGridCount(const int *__restrict__ grid_index, int *__restrict__ grid_counter, const unsigned int n)
{
    PX_CUDA_LOOP(i, n)
    {
        atomicAdd(&grid_counter[grid_index[i]], 1);
    }
}
__global__
void cudaSPHSort(int *__restrict__ pref_sum, const int *__restrict__ grid_index,
                 int *__restrict__ bucket, int *__restrict__ bucket_map,
                 const unsigned int n)
{
    PX_CUDA_LOOP(i, n)
    {
        auto index = grid_index[i];
        auto new_index = atomicAdd(&pref_sum[index], 1);
        bucket[new_index] = index;
        bucket_map[new_index] = i;
    }
}
__global__
void cudaSPHMakeBins(const int *__restrict__ bucket,
                     int *__restrict__ bucket_start,
                     const unsigned int n_grids,
                     const unsigned int n)
{
    PX_CUDA_LOOP(new_i, n)
    {
        auto cur_bucket = bucket[new_i];
        auto prev_bucket = new_i > 0 ? bucket[new_i-1] : 0;
        if (prev_bucket != cur_bucket)
        {
            for (int j = prev_bucket; j < cur_bucket; j++)
                bucket_start[j] = new_i;
        }
        if (new_i == n - 1)
        {
            for (int j = cur_bucket; j < n_grids; j++)
                bucket_start[j] = n;
        }
    }
}
__global__
void cudaSPHDensityAndPressure(const float3 *__restrict__ pos, const float *mass, const float *rest_density,
                               const int* __restrict__ bucket_map, const int *__restrict__ bucket_start,
                               const int3 *__restrict__ cell_index, const int3 resolution,
                               float *__restrict__ density, float *__restrict__ pressure,
                               const float smooth_len2, const float K_poly6, const float stiffness,
                               const unsigned int n, const int offset)
{
    PX_CUDA_LOOP(id, n)
    {
        auto rho = 0.f;
        auto i = id + offset;

        auto x_s = cell_index[i].x - 1;
        auto x_e = cell_index[i].x + 2;
        auto y_s = cell_index[i].y - 1;
        auto y_e = cell_index[i].y + 2;
        auto z_s = cell_index[i].z - 1;
        auto z_e = cell_index[i].z + 2;
        if (x_s < 0) x_s = 0;
        if (x_e > resolution.x) x_e = resolution.x;
        if (y_s < 0) y_s = 0;
        if (y_e > resolution.y) y_e = resolution.y;
        if (z_s < 0) z_s = 0;
        if (z_e > resolution.z) z_e = resolution.z;
        for (int ix = x_s; ix < x_e; ix++)
        {
            for (int iy = y_s; iy < y_e; iy++)
            {
                for (int iz = z_s; iz < z_e; iz++)
                {
                    auto index =
                            (ix * resolution.y + iy) * resolution.z + iz;


                    auto first = index > 0 ? bucket_start[index - 1] : 0;
                    auto last = bucket_start[index];
                    for (int neighbor = first; neighbor < last; neighbor++)
                    {
                        auto j = bucket_map[neighbor];
                        auto r = pos[i] - pos[j];
                        auto r2 = r.x*r.x+r.y*r.y+r.z*r.z;

                        if (r2 < smooth_len2)
                        {
                            auto rest_dist2 = smooth_len2 - r2;
                            rho = rho+mass[j]*rest_dist2*rest_dist2*rest_dist2;
                        }
                    }
                }
            }
        }
        rho *= K_poly6;
        auto pa =  stiffness * (rho - rest_density[i]);
        density[i] = rho;
        pressure[i] = pa > 0.f ? pa : 0.f;
    }
}
__global__
void cudaSPHAccleration(const float3 *__restrict__ pos, const float3 *__restrict__ vel,
                        const float *__restrict__ mass, const float *__restrict__ coef_viscosity,
                        const float *__restrict__ density, const float *__restrict__ pressure,
                        const int *__restrict__ bucket_map, const int *__restrict__ bucket_start, const int3 *__restrict__ cell_index, const int3 resolution,
                        float3 *__restrict__ acceleration, float *__restrict__ color,
                        const float smooth_len, const float smooth_len2,
                        const float3 gravity,
                        const float extra_coef_pressure,
                        const float extra_coef_viscosity,
                        const float neg_coef_surface_tension,
                        const float K_delta_spiky, const float K_delta_poly6, const float K_laplacian_poly6,
                        const unsigned int n, const int offset)
{
    PX_CUDA_LOOP(id, n)
    {
        auto i = id + offset;

        auto rho = density[i];
        auto p = pos[i];
        auto v = vel[i];
        float3 f_v, f_p, d_color;
        f_v.x = 0.f; f_p.x = 0.f; d_color.x = 0.f;
        f_v.y = 0.f; f_p.y = 0.f; d_color.y = 0.f;
        f_v.z = 0.f; f_p.z = 0.f; d_color.z = 0.f;
        auto d_color2 = 0.f;

        auto x_s = cell_index[i].x - 1;
        auto x_e = cell_index[i].x + 2;
        auto y_s = cell_index[i].y - 1;
        auto y_e = cell_index[i].y + 2;
        auto z_s = cell_index[i].z - 1;
        auto z_e = cell_index[i].z + 2;
        if (x_s < 0) x_s = 0;
        if (x_e > resolution.x) x_e = resolution.x;
        if (y_s < 0) y_s = 0;
        if (y_e > resolution.y) y_e = resolution.y;
        if (z_s < 0) z_s = 0;
        if (z_e > resolution.z) z_e = resolution.z;
        for (int ix = x_s; ix < x_e; ix++)
        {
            for (int iy = y_s; iy < y_e; iy++)
            {
                for (int iz = z_s; iz < z_e; iz++)
                {
                    auto index =
                            (ix * resolution.y + iy) * resolution.z + iz;
                    auto first = index > 0 ? bucket_start[index - 1] : 0;
                    auto last = bucket_start[index];
                    for (int neighbor = first; neighbor < last; neighbor++)
                    {
                        auto j = bucket_map[neighbor];
                        if (i == j) continue;

                        auto r = p - pos[j];
                        auto r2 = r.x*r.x+r.y*r.y+r.z*r.z;
                        if (r2 < smooth_len2)
                        {
                            auto dist = sqrtf(r2);
                            auto rest_len = smooth_len - dist;
                            auto rest_len2 = smooth_len2 - r2;
                            auto volume_rest_len = rest_len * mass[j]/density[j];

                            auto p_sum = pressure[i] + pressure[j];
//
                            if (p_sum != 0.f && dist != 0.f)
                                f_p = f_p + (p_sum*0.5f*volume_rest_len*rest_len/dist) * r;
                            f_v = f_v + (coef_viscosity[j]*volume_rest_len) * (vel[j] - v);
                            d_color = d_color + (volume_rest_len*rest_len2*rest_len2) * r;
                            d_color2 = d_color2 + volume_rest_len*rest_len2*(7.f*r2 - 3.f*smooth_len2);
                        }
                    }
                }
            }
        }

        d_color = K_delta_poly6  * d_color;
//        auto min = (abs(d_color.x) < abs(d_color.y) && abs(d_color.x) > 0.f) ? abs(d_color.x) : abs(d_color.y);
//        min = (min < abs(d_color.z) && min > 0.f) ? min : abs(d_color.z);
//        if (min > 0.f)
//        {
//            d_color.x = d_color.x/min;
//            d_color.y = d_color.y/min;
//            d_color.z = d_color.z/min;
            auto len_delta_color_sq = d_color.x*d_color.x + d_color.y*d_color.y + d_color.z*d_color.z;
            if (len_delta_color_sq > 0.f)
            {
                d_color = neg_coef_surface_tension*K_laplacian_poly6*d_color2/sqrtf(len_delta_color_sq) * d_color;
            }
//        }
//        color[i] = (d_color2*K_laplacian_poly6-1000)*1e-4f;
        // TODO surface tension visualization
        color[i] = 0.f;
        acceleration[i] = gravity + (d_color + K_delta_spiky*(extra_coef_pressure * f_p + extra_coef_viscosity * f_v)) / rho;
    }
}
__global__
void cudaSPHLeapfrog(float3 *__restrict__ pos, float3 *__restrict__ vel, float3 *__restrict__ vel_h, const float3 *__restrict__ acc,
                     const float3 lower_boundary, const float3 upper_boundary, const float particle_diameter,
                     const float dt,
                     const unsigned int n, const int offset)
{
    constexpr auto eps = 1e-6f;
    constexpr auto stiff = 5e4f;
    constexpr auto damp = 100.f;

#define BOUNDARY_CONSTRAINT(AXIS)                                               \
            {                                                                   \
                auto diff = particle_diameter - (p.AXIS - lower_boundary.AXIS); \
                if (diff > eps) a.AXIS = stiff * diff - damp * v.AXIS;          \
            }                                                                   \
            {                                                                   \
                auto diff = (upper_boundary.AXIS - p.AXIS) - particle_diameter; \
                if (diff < -eps) a.AXIS = stiff * diff - damp * v.AXIS;         \
            }

    PX_CUDA_LOOP(id, n)
    {
        auto i = id+offset;

        auto p = pos[i] + vel_h[i] * dt;
        auto v = vel[i];
        float3 a;
        a.x = 0.f;
        a.y = 0.f;
        a.z = 0.f;

        BOUNDARY_CONSTRAINT(x);
        BOUNDARY_CONSTRAINT(y);
        BOUNDARY_CONSTRAINT(z);

        auto dv = (acc[i]+ a)*dt;

        vel[i]   = vel_h[i] + dv*0.5f;
        vel_h[i] = vel_h[i] + dv;
        pos[i]   = pos[i] + vel_h[i]*dt;
    }
}


void scene::SPHScene::cudaInit(const float *mass, const float *rest_density, const float *coef_viscosity,
                               unsigned int capacity)
{
    if (cuda_buffer.streams == nullptr && cuda_buffer.n_streams > 0)
    {
        cuda_buffer.streams = new cudaStream_t[cuda_buffer.n_streams];
        for (auto i = 0; i < cuda_buffer.n_streams; i++)
            PX_CUDA_CHECK(cudaStreamCreateWithFlags(&cuda_buffer.streams[i], cudaStreamNonBlocking));
    }
    if (capacity > cuda_buffer.capacity)
    {
        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.acceleration, sizeof(float3)*capacity));
        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.density, sizeof(float)*capacity));
        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.pressure, sizeof(float)*capacity));
        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.grid_index, sizeof(int)*capacity));
        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.cell_index, sizeof(int3)*capacity));
        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.bucket, sizeof(int)*capacity));
        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.bucket_map, sizeof(int)*capacity));

        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.grid_counter, sizeof(int)*n_grids));
        PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.bucket_start, sizeof(int)*n_grids));

        if (cuda_buffer.capacity > 0)
        {
            void *mass = nullptr;
            void *rest_density = nullptr;
            void *coef_viscosity = nullptr;
            void *velocity = nullptr;
            void *velocity_h = nullptr;
            PX_CUDA_CHECK(cudaMalloc(&mass, sizeof(float)*capacity));
            PX_CUDA_CHECK(cudaMalloc(&rest_density, sizeof(float)*capacity));
            PX_CUDA_CHECK(cudaMalloc(&coef_viscosity, sizeof(float)*capacity));
            PX_CUDA_CHECK(cudaMalloc(&velocity, sizeof(float3)*capacity));
            PX_CUDA_CHECK(cudaMalloc(&velocity_h, sizeof(float3)*capacity));

            PX_CUDA_CHECK(cudaMemcpy(velocity, cuda_buffer.mass, sizeof(float3)*cuda_buffer.capacity, cudaMemcpyDeviceToDevice));
            PX_CUDA_CHECK(cudaMemcpy(velocity, cuda_buffer.velocity, sizeof(float3)*cuda_buffer.capacity, cudaMemcpyDeviceToDevice));
            PX_CUDA_CHECK(cudaMemcpy(velocity_h, cuda_buffer.velocity_h, sizeof(float3)*cuda_buffer.capacity, cudaMemcpyDeviceToDevice));

            PX_CUDA_CHECK(cudaFree(cuda_buffer.mass));
            PX_CUDA_CHECK(cudaFree(cuda_buffer.rest_density));
            PX_CUDA_CHECK(cudaFree(cuda_buffer.coef_viscosity));
            PX_CUDA_CHECK(cudaFree(cuda_buffer.velocity));
            PX_CUDA_CHECK(cudaFree(cuda_buffer.velocity_h));

            cuda_buffer.mass = reinterpret_cast<float *>(mass);
            cuda_buffer.rest_density = reinterpret_cast<float *>(rest_density);
            cuda_buffer.coef_viscosity = reinterpret_cast<float *>(coef_viscosity);
            cuda_buffer.velocity = reinterpret_cast<float3 *>(velocity);
            cuda_buffer.velocity_h = reinterpret_cast<float3 *>(velocity_h);
        }
        else
        {
            PX_CUDA_CHECK(cudaFree(cuda_buffer.mass));
            PX_CUDA_CHECK(cudaFree(cuda_buffer.rest_density));
            PX_CUDA_CHECK(cudaFree(cuda_buffer.coef_viscosity));
            PX_CUDA_CHECK(cudaFree(cuda_buffer.velocity));
            PX_CUDA_CHECK(cudaFree(cuda_buffer.velocity_h));

            PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.mass, sizeof(float)*capacity));
            PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.rest_density, sizeof(float)*capacity));
            PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.coef_viscosity, sizeof(float)*capacity));
            PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.velocity, sizeof(float3)*capacity));
            PX_CUDA_CHECK(cudaMalloc(&cuda_buffer.velocity_h, sizeof(float3)*capacity));

            PX_CUDA_CHECK(cudaMemset(cuda_buffer.velocity, 0, sizeof(float3)*capacity));
            PX_CUDA_CHECK(cudaMemset(cuda_buffer.velocity_h, 0, sizeof(float3)*capacity));
        }
        cuda_buffer.capacity = capacity;
    }
    PX_CUDA_CHECK(cudaMemcpy(cuda_buffer.mass, mass, sizeof(float)*capacity, cudaMemcpyHostToDevice));
    PX_CUDA_CHECK(cudaMemcpy(cuda_buffer.rest_density, rest_density, sizeof(float)*capacity, cudaMemcpyHostToDevice));
    PX_CUDA_CHECK(cudaMemcpy(cuda_buffer.coef_viscosity, coef_viscosity, sizeof(float)*capacity, cudaMemcpyHostToDevice));
}

void scene::SPHScene::cudaAppend(float3* pos_buffer, unsigned int offset,
                                 const float3* pos, const float3* vel, const float* mass,
                                 const float* rest_density, const float *coef_viscosity,
                                 int n)
{
    PX_CUDA_CHECK(cudaMemcpy(pos_buffer + offset, pos, sizeof(float3)*n, cudaMemcpyHostToDevice));
    PX_CUDA_CHECK(cudaMemcpy(cuda_buffer.velocity + offset, vel, sizeof(float3)*n, cudaMemcpyHostToDevice));
    PX_CUDA_CHECK(cudaMemcpy(cuda_buffer.velocity_h + offset, vel, sizeof(float3)*n, cudaMemcpyHostToDevice));
    PX_CUDA_CHECK(cudaMemcpy(cuda_buffer.mass + offset, mass, sizeof(float)*n, cudaMemcpyHostToDevice));
    PX_CUDA_CHECK(cudaMemcpy(cuda_buffer.rest_density + offset, rest_density, sizeof(float)*n, cudaMemcpyHostToDevice));
    PX_CUDA_CHECK(cudaMemcpy(cuda_buffer.coef_viscosity + offset, coef_viscosity, sizeof(float)*n, cudaMemcpyHostToDevice));
}

void scene::SPHScene::cudaDrop()
{
    PX_CUDA_CHECK(cudaFree(cuda_buffer.mass));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.rest_density));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.coef_viscosity));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.velocity));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.velocity_h));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.acceleration));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.density));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.pressure));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.grid_index));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.cell_index));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.bucket));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.bucket_map));

    cuda_buffer.mass = nullptr;
    cuda_buffer.rest_density = nullptr;
    cuda_buffer.coef_viscosity = nullptr;
    cuda_buffer.velocity = nullptr;
    cuda_buffer.velocity_h = nullptr;
    cuda_buffer.acceleration = nullptr;
    cuda_buffer.density = nullptr;
    cuda_buffer.pressure = nullptr;
    cuda_buffer.grid_index = nullptr;
    cuda_buffer.cell_index = nullptr;
    cuda_buffer.bucket = nullptr;
    cuda_buffer.bucket_map = nullptr;

    PX_CUDA_CHECK(cudaFree(cuda_buffer.grid_counter));
    PX_CUDA_CHECK(cudaFree(cuda_buffer.bucket_start));
    cuda_buffer.bucket_start = nullptr;
    cuda_buffer.grid_counter = nullptr;

    if (cuda_buffer.streams != nullptr)
    {
        for (int i = 0; i < cuda_buffer.n_streams; i++)
            PX_CUDA_CHECK(cudaStreamDestroy(cuda_buffer.streams[i]));
        delete cuda_buffer.streams;
        cuda_buffer.streams = nullptr;
    }

    cuda_buffer.capacity = 0;
}

void scene::SPHScene::cudaUpdate(float3 *pos, float *color, float dt)
{
    const static float3 zero3 = make_float3(0.f, 0.f, 0.f);

    // see http://on-demand.gputechconf.com/gtc/2014/presentations/S4117-fast-fixed-radius-nearest-neighbor-gpu.pdf
    // this implementation is based on the above doc, but not exactly the same

    auto n_particles = particles();
    if (n_particles < 1) return;

    auto n_particles_per_stream = cuda_buffer.n_streams == 0 ? 0 : n_particles / cuda_buffer.n_streams;
    auto remaining = n_particles - n_particles_per_stream*cuda_buffer.n_streams;
    auto blocks = cuda::blocks(n_particles);
    auto blocks_per_stream = cuda::blocks(n_particles_per_stream);
    auto blocks_remaining = cuda::blocks(remaining);


    int offset;
#define STREAM_CALL(FUNC, args...)                                                              \
    {                                                                                           \
        offset = 0;                                                                             \
        for (auto i = 0; i < cuda_buffer.n_streams; i++)                                        \
        {                                                                                       \
            FUNC<<<blocks_per_stream, PX_CUDA_THREADS_PER_BLOCK, 0, cuda_buffer.streams[i]>>>(  \
                    args,                                                                       \
                    n_particles_per_stream, offset                                              \
            );                                                                                  \
            offset += n_particles_per_stream;                                                   \
        }                                                                                       \
        if (remaining > 0)                                                                      \
            FUNC<<<blocks_remaining, PX_CUDA_THREADS_PER_BLOCK, 0>>>(                           \
                    args,                                                                       \
                    remaining, offset                                                           \
            );                                                                                  \
        PX_CUDA_CHECK(cudaDeviceSynchronize());                                                 \
    };

//    TIC(0);
    cudaSPHClearCounter<<<cuda::blocks(n_grids), PX_CUDA_THREADS_PER_BLOCK>>>(
            cuda_buffer.grid_counter, n_grids
    );
//    cudaStream_t s;
//    cudaStreamCreateWithFlags(&s, cudaStreamNonBlocking);
//    cudaMemsetAsync(cuda_buffer.grid_counter, 0, sizeof(int)*n_grids, s);
//    PX_CUDA_CHECK(cudaDeviceSynchronize());
//    TOC(0)


//    TIC(1);
    STREAM_CALL(cudaSPHPosition2Grid,
                pos, cuda_buffer.grid_index, cuda_buffer.cell_index,
                lower_boundary, smooth_len, resolution);
//    TOC(1);

//    TIC(2);
    cudaSPHGridCount<<<blocks, PX_CUDA_THREADS_PER_BLOCK>>>(
            cuda_buffer.grid_index, cuda_buffer.grid_counter, n_particles
    );
//    PX_CUDA_CHECK(cudaDeviceSynchronize());
//    TOC(2)
//    TIC(3);
    thrust::device_ptr<int> grid_ptr(cuda_buffer.grid_counter);
    thrust::device_vector<int> counter(n_grids);
    thrust::exclusive_scan(grid_ptr, grid_ptr+n_grids, counter.begin());
    auto pref_sum = thrust::raw_pointer_cast(&counter[0]);
//    TOC(3)
//    TIC(4)
    cudaSPHSort<<<blocks, PX_CUDA_THREADS_PER_BLOCK>>>(
            pref_sum, cuda_buffer.grid_index, cuda_buffer.bucket,
            cuda_buffer.bucket_map, n_particles
    );
//    PX_CUDA_CHECK(cudaDeviceSynchronize());
//    TOC(4)
//    TIC(5)
    cudaSPHMakeBins<<<blocks, PX_CUDA_THREADS_PER_BLOCK>>>(
           cuda_buffer.bucket, cuda_buffer.bucket_start,
           n_grids, n_particles
    );
    PX_CUDA_CHECK(cudaDeviceSynchronize());
//    TOC(5)
//
//
//    TIC(6)
    STREAM_CALL(cudaSPHDensityAndPressure,
                pos, cuda_buffer.mass, cuda_buffer.rest_density,
                cuda_buffer.bucket_map, cuda_buffer.bucket_start, cuda_buffer.cell_index, resolution,
                cuda_buffer.density, cuda_buffer.pressure,
                smooth_len2, K_poly6, stiffness);
//    TOC(6)
//    TIC(7)
    STREAM_CALL(cudaSPHAccleration,
                pos, cuda_buffer.velocity, cuda_buffer.mass, cuda_buffer.coef_viscosity,
                cuda_buffer.density, cuda_buffer.pressure,
                cuda_buffer.bucket_map, cuda_buffer.bucket_start, cuda_buffer.cell_index, resolution,
                cuda_buffer.acceleration, color,
                smooth_len, smooth_len2,
                (scene_param.gravity_force ? gravity : zero3),
                (scene_param.pressure_force ? 1.f : 0.f),
                (scene_param.viscosity_force ? 1.f : 0.f),
                (scene_param.surface_tension ? neg_coef_surface_tension : 0.f),
                K_delta_spiky, K_delta_poly6, K_laplacian_poly6);
//    TOC(7)
//    TIC(8)
    STREAM_CALL(cudaSPHLeapfrog,
                pos, cuda_buffer.velocity, cuda_buffer.velocity_h, cuda_buffer.acceleration,
                lower_boundary, upper_boundary, particle_diameter,
                dt);
//    TOC(8)
}

