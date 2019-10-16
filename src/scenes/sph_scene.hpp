#ifndef PX_CG_SCENES_SPH_SCENE_HPP
#define PX_CG_SCENES_SPH_SCENE_HPP

#include "scenes/forward_rendering_scene.hpp"
#include "shaders/forward_phong.hpp"

namespace px { namespace scene {
class SPHScene;
}}

#include "utils/shape_generator.hpp"
#include "items/ball.hpp"
#include "utils/random.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/gtx/norm.hpp>

#if defined(__APPLE__)
#include <Accelerate/Accelerate.h>
#else
#include <cblas.h>
#endif

#include <iostream>
#include <chrono>

#define TIC(id) \
    auto _tic_##id = std::chrono::system_clock::now();
#define TOC(id) \
    auto _toc_##id = std::chrono::system_clock::now(); \
    std::cout << "[Info] Process time " << id << ": " \
              << std::chrono::duration_cast<std::chrono::milliseconds>(_toc_##id - _tic_##id).count() \
              << "ms" << std::endl;

#include <array>
#include <numeric>
#include <cstring>
#if defined(USE_CUDA)
#include <cuda_gl_interop.h>
#include <items/pillar.hpp>
#include "utils/cuda.hpp"
#endif

class px::scene::SPHScene : public ForwardRenderingScene {
public:
    enum class Scenery
    {
        Empty,
        Normal,
        SurfaceTension
    } scenery = Scenery::SurfaceTension;

    struct SceneParameter
    {
        int n_obstacles = 5;
        bool viscosity_force = true;
        bool pressure_force = true;
        bool surface_tension = true;
        bool gravity_force = true;

        float step_size = 0.0008f;
        int max_steps = 5;
        glm::vec3 gravity = glm::vec3(0.f, -9.8f, 0.f);
        SceneParameter(float step_size, int max_steps, glm::vec3 const &gravity)
                : step_size(step_size), max_steps(max_steps), gravity(gravity)
        {}
        SceneParameter() = default;
    } scene_param;

    bool show_box = true;
    bool show_obs = true;
    bool tap_open = false;

#if defined(USE_CUDA)
    bool use_cuda = true;

    void cudaInit(const float *mass, const float *rest_density, const float *coef_viscosity,
                  unsigned int capacity);
    void cudaUpdate(float3 *pos, float *color, float dt);
    void cudaDrop();
    void cudaAppend(float3* pos_buffer, unsigned int offset,
                    const float3* pos, const float3* vel, const float* mass,
                    const float* rest_density, const float *coef_viscosity,
                    int n);

#endif

    SPHScene();
    explicit SPHScene(const SceneParameter &scene_param);
    ~SPHScene() override;

    void init() override;
    void restart();
    void update(float dt) override;
    void render() override;

    int particles();
    bool isUseCuda();

protected:

    float smooth_len;
    float mass;
    float stiffness = 1.5;
    float rest_density = 1000;
    float coef_viscosity = 0.1;
    float neg_coef_surface_tension = -0.08;
    float particle_diameter;
    
    float smooth_len2;
    float K_poly6;
    float K_delta_poly6;
    float K_laplacian_poly6;
    float K_delta_spiky;

#if defined(USE_CUDA)
    float3 gravity;
    float3 lower_boundary;
    float3 upper_boundary;
    int3 resolution;
    int n_grids;
#endif
    class impl;
    std::unique_ptr<impl> pimpl;
};


#endif // PX_CG_SCENES_SPH_SCENE_HPP
