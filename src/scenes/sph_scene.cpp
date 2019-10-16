#include "sph_scene.hpp"
#include "items/floor.hpp"
#include "items/sphere.hpp"

#include "utils/random.hpp"
#include "utils/shape_generator.hpp"

#include <algorithm>

using namespace px;


    class scene::SPHScene::impl {
    public:

        struct ObstacleSphere
        {
            glm::vec3 pos;
            float radius;
            ObstacleSphere()
                    :pos(glm::vec3(1.f)), radius(1.f)
            {}
            ObstacleSphere(glm::vec3 const &p, float r)
                    : pos(p), radius(r)
            {}
        };
        struct ObstacleCylinder
        {
            glm::vec2 pos;
            float top;
            float bottom;
            float radius;
            ObstacleCylinder()
                    :pos(glm::vec2(1.f)), top(1.f), bottom(-1.f), radius(1.f)
            {}
            ObstacleCylinder(glm::vec2 const &p, float t, float b, float r)
                    : pos(p), top(t), bottom(b), radius(r)
            {}
        };

        class ObstacleCylinderShader : public shader::ForwardPhong
        {
        public:
            item::Material material;
            std::vector<item::Pillar> objs;

            explicit ObstacleCylinderShader()
                    : ForwardPhong()
            {
                material.ambient = glm::vec3(0.3f, 0.f, 0.f);
                material.diffuse = glm::vec3(0.5f, 0.f, 0.f);
                material.specular = glm::vec3(1.f);
                material.shininess = 32.f;
            }
            void init()
            {
                ForwardPhong::init();
            }
            void restart(ObstacleCylinder *obs, int n)
            {
                objs.clear();
                objs.reserve(n);
                for (auto i = 0; i < n; i++)
                {
                    objs.emplace_back(material);
                    objs.back().pos(glm::vec3(obs[i].pos.x, (obs[i].bottom+obs[i].top)*.5f, obs[i].pos.y));
                    objs.back().scal(glm::vec3(obs[i].radius, (obs[i].top-obs[i].bottom)*.5f, obs[i].radius));
                    objs.back().init();
                }
            }
            void render()
            {
                activate(true);
                for (auto &o : objs)
                    o.render(*this);
                activate(false);
            }

        } obstacle_shader;

        class BoxShader : public Shader
        {
        public:
            const char *STRING_VS = "#version 330 core\n"
                                    "layout(location = 0) in vec3 position;"
                                    ""
                                    "layout (std140) uniform SceneCamera"
                                    "{"
                                    "    mat4 view;"
                                    "    mat4 projection;"
                                    "    vec3 camera_position;"
                                    "};"
                                    ""
                                    "void main()"
                                    "{"
                                    "   gl_Position = projection * view * vec4(position, 1.f);"
                                    "}";
            const char *STRING_FS = "#version 330 core\n"
                                    ""
                                    "out vec4 gColor;"
                                    ""
                                    "void main(){"
                                    "   gColor = vec4(.6f, .6f, .6f, 1.f);"
                                    "}";

            unsigned int vao, vbo[2];

            BoxShader()
                    : Shader(), vao(0), vbo{0}
            {}

            ~BoxShader() override
            {
                glDeleteVertexArrays(1, &vao);
                glDeleteBuffers(2, vbo);
            }

            void init()
            {
                glGenVertexArrays(1, &vao);
                glGenBuffers(2, vbo);
                Shader::init(STRING_VS, STRING_FS);

                // 0, x_l, y_l, z_l
                // 1, x_u, y_l, z_l
                // 2, x_u, y_l, z_u
                // 3, x_l, y_l, z_u
                // 4, x_l, y_u, z_u
                // 5, x_l, y_u, z_l
                // 6, x_u, y_u, z_l
                // 7, x_u, y_u, z_u
                std::uint8_t link[] = {
                        0, 1,
                        1, 2,
                        2, 3,
                        3, 0,
                        4, 5,
                        5, 6,
                        6, 7,
                        7, 4,
                        0, 5,
                        1, 6,
                        2, 7,
                        3, 4,
                };

                activate(true);
                glBindVertexArray(vao);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint8_t)*24, link, GL_STATIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24, nullptr, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);   // position
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

                activate(false);
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }

            void restart(glm::vec3 const &lower, glm::vec3 const &upper)
            {
                float vertex[] = {
                        lower.x, lower.y, lower.z,  // 0, x_l, y_l, z_l
                        upper.x, lower.y, lower.z,  // 1, x_u, y_l, z_l
                        upper.x, lower.y, upper.z,  // 2, x_u, y_l, z_u
                        lower.x, lower.y, upper.z,  // 3, x_l, y_l, z_u
                        lower.x, upper.y, upper.z,  // 4, x_l, y_u, z_u
                        lower.x, upper.y, lower.z,  // 5, x_l, y_u, z_l
                        upper.x, upper.y, lower.z,  // 6, x_u, y_u, z_l
                        upper.x, upper.y, upper.z   // 7, x_u, y_u, z_u
                };
                glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*24, vertex);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

            void render()
            {
                activate(true);
                glBindVertexArray(vao);
                glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, nullptr);
                activate(false);
            }

        } box_shader;
        class ParticleShader : public Shader
        {
        public:
            const char *VS =
            #include "shaders/glsl/sph.vs"
            ;
            const char *FS =
            #include "shaders/glsl/sph.fs"
            ;

            item::Material material1;
            item::Material material2;
            float radius = 1.f;

            unsigned int vao;
            unsigned int vbo[5];

            ParticleShader()
                    : Shader(), vao(0), vbo{0}
            {
                material1.ambient = glm::vec3(0.2588f, 0.5255f, 0.9569f)*.2f;
                material1.diffuse = glm::vec3(0.2588f, 0.5255f, 0.9569f)*.5f;
                material1.specular = glm::vec3(.2f);
                material1.shininess = 16.f;

                material2.ambient = glm::vec3(0.0f, 0.3f, 0.f);
                material2.diffuse = glm::vec3(0.0f, 0.5f, 0.f);
                material2.specular = glm::vec3(.2f);
                material2.shininess = 16.f;
            }
            ~ParticleShader() override
            {
                glDeleteVertexArrays(1, &vao);
                glDeleteBuffers(5, vbo);
            }

            void init()
            {
                Shader::init(VS, FS);

                glGenVertexArrays(1, &vao);
                glGenBuffers(5, vbo);


                glBindVertexArray(vao);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);

                n_instances1_ = 0;
                n_instances2_ = 0;
                // pos
                glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribDivisor(0, 1);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                // color
                glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
                glEnableVertexAttribArray(3);
                glVertexAttribDivisor(3, 0);
                glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

                n_indices_ = 0;
                // norm
                glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glEnableVertexAttribArray(1);
                glVertexAttribDivisor(1, 0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                // vertex
                glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
                glEnableVertexAttribArray(2);
                glVertexAttribDivisor(2, 0);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);


                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                // activate(true);
                // set("model", Item::IDENTITY_MAT);
                // activate(false);
            }

            void restart()
            {
                std::vector<float> vertices, uv, norm, tangent;
                std::vector<unsigned short> vertex_indices;
//                std::tie(vertices, vertex_indices, uv, norm, tangent) =
                generator::sphereWithNormUVTangle(6, radius, vertices, vertex_indices, uv, norm, tangent);

                n_indices_ = vertex_indices.size();
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*vertex_indices.size(), vertex_indices.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float)*norm.size(), norm.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                n_instances1_ = 0;
                n_instances2_ = 0;
            }

            void setInstances(unsigned int n)
            {
                if (n_instances1_ + n_instances2_ + n > n_capacity_)
                {
                    // FIXME number of instance beyond the capacity
                }
                n_instances2_ = n - n_instances1_;
            }

            bool setInstances(std::vector<glm::vec3> const &pos, unsigned int n)
            {
                auto updated = false;
                if (n_instances1_ == 0)
                    n_instances1_ = n;
                else
                    n_instances2_ = n - n_instances1_;
                if (pos.capacity() > n_capacity_)
                {
                    n_capacity_ = pos.capacity();

                    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*n_capacity_,
                                 nullptr,
                                 GL_STREAM_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*n_capacity_,
                                 nullptr,
                                 GL_STREAM_DRAW);

                    updated = true;

                }
                glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*n,
                                    reinterpret_cast<const float*>(pos.data()));
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                return updated;
            }

            void render(bool double_color = true)
            {
                activate(true);
                glBindVertexArray(vao);

                set("material.ambient", material1.ambient);
                set("material.diffuse", material1.diffuse);
                set("material.specular", material1.specular);
                set("material.shininess", material1.shininess);
                glDrawElementsInstancedBaseInstance(GL_TRIANGLES, n_indices_, GL_UNSIGNED_SHORT, nullptr, n_instances1_, 0);

                if (double_color)
                {
                set("material.ambient", material2.ambient);
                set("material.diffuse", material2.diffuse);
                set("material.specular", material2.specular);
                set("material.shininess", material2.shininess);
                }
                glDrawElementsInstancedBaseInstance(GL_TRIANGLES, n_indices_, GL_UNSIGNED_SHORT, nullptr, n_instances2_, n_instances1_);

                glBindVertexArray(0);
                activate(false);
            }

        private:
            unsigned int n_instances1_ = 0;
            unsigned int n_instances2_ = 0;
            unsigned int n_indices_ = 0;
            unsigned int n_capacity_ = 0;
        } particle_shader;

        SPHScene &scene;
#if defined(USE_CUDA)
        bool use_cuda;
#endif

        std::vector<glm::vec3> position;
        std::vector<glm::vec3> velocity;
        std::vector<glm::vec3> velocity_h;
        std::vector<glm::vec3> acceleration;

        std::vector<glm::ivec3> grid_index;
        struct Dist {
            int index;
            float dist;
            float dist2;
            float rest_dist;
            float rest_dist2;
            Dist()
            {}
            Dist(int index, float dist, float dist2, float rest_dist, float rest_dist2)
                    : index(index), dist(dist), dist2(dist2), rest_dist(rest_dist), rest_dist2(rest_dist2)
            {}
        };
        std::vector<std::vector<Dist> > neighbors;
        std::vector<float> density;
        std::vector<float> pressure;

        std::vector<float> rest_density_state;
        std::vector<float> coef_viscosity_state;
        std::vector<float> mass_state;

        std::vector<ObstacleCylinder> obstacles;


        int n_particles;
        glm::vec3 lower_boundary;  // = glm::vec3(-.175f, 0.f, -.1f);
        glm::vec3 upper_boundary;  // = glm::vec3(.175f, .2f, .1f);
        glm::ivec3 resolution;

        bool first_step;

#if defined(USE_CUDA)
        struct cudaGraphicsResource *cuda_res[2];
#endif
        explicit impl(SPHScene &scene)
            : scene(scene)
#if defined(USE_CUDA)
, cuda_res{nullptr}
#endif
        {}

        ~impl()
        {
#if defined(USE_CUDA)
            scene.cudaDrop();
            if (cuda_res[0] != nullptr)
                cudaGraphicsUnregisterResource(cuda_res[0]);
            if (cuda_res[1] != nullptr)
                cudaGraphicsUnregisterResource(cuda_res[1]);
#endif
        }

        void init()
        {
            particle_shader.init();
            box_shader.init();
            obstacle_shader.init();
        }

        void restart()
        {
            scene.stiffness = 1.5f;
            scene.rest_density = 1000.f;
            scene.coef_viscosity = .1f;
            scene.neg_coef_surface_tension = -0.08f;

#if defined(USE_CUDA)
            use_cuda = scene.use_cuda;
            if (use_cuda)
            {
                n_particles = 160000;
                scene.smooth_len = 0.005;
            }
            else
#endif
            {
                n_particles = 50000;
                scene.smooth_len = 0.005;
            }
            auto buffer_cap = n_particles*2;
            first_step = true;
            gen_remaining = 0.f;
            scene.smooth_len2 = scene.smooth_len*scene.smooth_len;
            scene.mass = scene.smooth_len2*scene.smooth_len * scene.rest_density;
            particle_shader.radius = scene.smooth_len*0.25f;


            auto pi_r_9 = static_cast<float>(M_PI*std::pow(scene.smooth_len, 9));
            scene.K_poly6 = 315/(64*pi_r_9);
            scene.K_delta_poly6 = -945.f/(32.f*pi_r_9);
            scene.K_laplacian_poly6 = 945.f/(8.f*pi_r_9);
            scene.K_delta_spiky = 45/(static_cast<float>(M_PI*std::pow(scene.smooth_len, 6)));

            scene.particle_diameter = particle_shader.radius + particle_shader.radius;

            float gap_x, gap_y, gap_z;
            if (scene.scenery == Scenery::SurfaceTension)
            {
                gap_x = scene.smooth_len*.8f;
                gap_y = scene.smooth_len;
                gap_z = scene.smooth_len*.8f;
            }
            else
            {
                gap_x = scene.smooth_len*.6f;
                gap_y = scene.smooth_len*.6f;
                gap_z = scene.smooth_len*.8f;
            }

            auto n_y = static_cast<int>(std::pow(n_particles, 1.f/3.f));
            auto n_x = n_y;
            auto n_z = n_y;

            auto x_len = gap_x*n_x*3.5f+scene.smooth_len*2.f;
            auto y_len = gap_y*n_y*2.f+scene.smooth_len*3.f;
            auto z_len = gap_z*n_z*1.5f+scene.smooth_len;
            lower_boundary.x = -x_len*0.5f;
            lower_boundary.y = 0.f;
            lower_boundary.z = -z_len*0.5f;
            upper_boundary.x = x_len*0.5f;
            upper_boundary.y = y_len;
            upper_boundary.z = z_len*0.5f;

            position.clear();
            position.resize(buffer_cap);
            position.shrink_to_fit();
            position.clear();
            if (scene.scenery == Scenery::Normal)
            {
                for (int i = 0; i < n_x; i++)
                {
                    for (int j = 0; j < n_y; j++)
                    {
                        for (int k = 0; k < n_z; k++)
                        {
                            auto index = (i * n_y + j) * n_z + k;
                            if (index >= n_particles) return;
                            position.emplace_back(
                                    i * gap_x + lower_boundary.x +
                                    scene.smooth_len +
                                    rnd_np() * scene.smooth_len,
                                    j * gap_y + lower_boundary.y +
                                    scene.smooth_len * 2.f +
                                    rnd_np() * scene.smooth_len,
                                    (k - n_z * 0.5f) * gap_z +
                                    rnd_np() * scene.smooth_len
                                                 );
                        }
                    }
                }
            }
            else if (scene.scenery == Scenery::SurfaceTension)
            {
                scene.scene_param.viscosity_force = false;
                scene.scene_param.pressure_force = false;
                scene.scene_param.surface_tension = true;
                scene.scene_param.gravity_force = false;

                for (int i = 0; i < n_x; i++)
                {
                    for (int j = 2; j < n_y; j++)
                    {
                        for (int k = 0; k < n_z; k++)
                        {
                            auto index = (i*n_y + j)*n_z + k;
                            if (index >= n_particles) return;
                            position.emplace_back(
                                    (i - n_x*0.5f)*gap_x,
                                    (j - n_y*0.5f)*gap_y + (upper_boundary.y+lower_boundary.y)*0.5f,
                                    (k - n_z*0.5f)*gap_z);
                        }
                    }
                }
            }
            obstacles.clear();
#if defined(USE_CUDA)
            if (!use_cuda)
#endif
            {
                if (scene.scenery != Scenery::SurfaceTension)
                {
                    auto gap_x = upper_boundary.x - lower_boundary.x;
                    auto gap_z = upper_boundary.z - lower_boundary.z;
                    auto half_height = lower_boundary.y + (upper_boundary.y-lower_boundary.y)*0.5f;
                    for (auto i = 0; i < scene.scene_param.n_obstacles; i++)
                    {
                        auto r = scene.smooth_len*.75f * (1 + rnd());
                        auto p = glm::vec2(lower_boundary.x + gap_x*rnd(),
                                           lower_boundary.z + gap_z*rnd());
                        auto not_overlap = true;
                        for (auto &o : obstacles)
                        {
                            if (glm::distance(p, o.pos) < o.radius + r)
                            {
                                not_overlap = false;
                                break;
                            }
                        }
                        if (not_overlap)
                        {
                            auto t = half_height + half_height*rnd();
                            obstacles.emplace_back(p, t, lower_boundary.y, r);
                        }
                    }
                }
            }

            n_particles = position.size();

            particle_shader.restart();
            box_shader.restart(lower_boundary, upper_boundary);
            obstacle_shader.restart(obstacles.data(), obstacles.size());

#if defined(USE_CUDA)
            scene.cudaDrop();
            if (use_cuda)
            {
                scene.lower_boundary.x = lower_boundary.x;
                scene.lower_boundary.y = lower_boundary.y;
                scene.lower_boundary.z = lower_boundary.z;
                scene.upper_boundary.x = upper_boundary.x;
                scene.upper_boundary.y = upper_boundary.y;
                scene.upper_boundary.z = upper_boundary.z;
                scene.resolution.x = static_cast<int>((upper_boundary.x - lower_boundary.x) / scene.smooth_len + 1);
                scene.resolution.y = static_cast<int>((upper_boundary.y - lower_boundary.y) / scene.smooth_len + 1);
                scene.resolution.z = static_cast<int>((upper_boundary.z - lower_boundary.z) / scene.smooth_len + 1);
                scene.n_grids = scene.resolution.x * scene.resolution.y * scene.resolution.z;
                scene.gravity.x = scene.scene_param.gravity.x;
                scene.gravity.y = scene.scene_param.gravity.y;
                scene.gravity.z = scene.scene_param.gravity.z;

                if (particle_shader.setInstances(position, n_particles))
                {
                    if (cuda_res[0] != nullptr)
                        PX_CUDA_CHECK(cudaGraphicsUnregisterResource(cuda_res[0]));
                    if (cuda_res[1] != nullptr)
                        PX_CUDA_CHECK(cudaGraphicsUnregisterResource(cuda_res[1]));
                    PX_CUDA_CHECK(cudaGraphicsGLRegisterBuffer(
                            &cuda_res[0], particle_shader.vbo[0],
                            cudaGraphicsRegisterFlagsNone
                                                              ));
                    PX_CUDA_CHECK(cudaGraphicsGLRegisterBuffer(
                            &cuda_res[1], particle_shader.vbo[3],
                            cudaGraphicsRegisterFlagsNone
                                                              ));
                }
            }
            else
            {
#endif
                resolution.x = static_cast<int>((upper_boundary.x - lower_boundary.x) / scene.smooth_len + 1);
                resolution.y = static_cast<int>((upper_boundary.y - lower_boundary.y) / scene.smooth_len + 1);
                resolution.z = static_cast<int>((upper_boundary.z - lower_boundary.z) / scene.smooth_len + 1);

                density.reserve(buffer_cap);
                pressure.reserve(buffer_cap);
                acceleration.reserve(buffer_cap);
                velocity.reserve(buffer_cap);
                velocity_h.reserve(buffer_cap);
                grid_index.reserve(buffer_cap);
                neighbors.reserve(buffer_cap);
                density.reserve(buffer_cap);

                velocity.resize(position.size());
                velocity_h.resize(position.size());
                density.resize(position.size());
                pressure.resize(position.size());
                acceleration.resize(position.size());
                grid_index.resize(position.size());
                neighbors.resize(position.size());

                for (auto &n : neighbors) n.reserve(200);
                std::memset(velocity.data(), 0,
                            sizeof(glm::vec3) * velocity.size());
                std::memset(velocity_h.data(), 0,
                            sizeof(glm::vec3) * velocity_h.size());

#if defined(USE_CUDA)
            }
#endif

            mass_state.reserve(buffer_cap);
            rest_density_state.reserve(buffer_cap);
            coef_viscosity_state.reserve(buffer_cap);

            mass_state.clear();
            rest_density_state.clear();
            coef_viscosity_state.clear();

            rest_density_state.resize(position.size(), scene.rest_density);
            coef_viscosity_state.resize(position.size(), scene.coef_viscosity);
            mass_state.resize(position.size(), scene.mass);

#if defined(USE_CUDA)
            if (use_cuda)
            {
                scene.cudaInit(mass_state.data(), rest_density_state.data(), coef_viscosity_state.data(),
                               position.capacity());
            }
#endif


            std::cout << scene.mass << std::endl;
           if (scene.scenery != Scenery::Empty)
            {
                scene.rest_density = 0.f;
                scene.coef_viscosity = 10.f;
                scene.mass = .0005f;
            }
        }

        void dynamics(float dt)
        {
            std::vector<std::vector<int> > grid;

            glm::vec3 upper_bound = glm::vec3(-std::numeric_limits<float>::max());
            glm::vec3 lower_bound = // lower_boundary;
                    glm::vec3(std::numeric_limits<float>::max());
            for (int i = 0; i < n_particles; i++)
            {
                auto &p = position[i];
                if (p.x > upper_bound.x) upper_bound.x = p.x;
                if (p.y > upper_bound.y) upper_bound.y = p.y;
                if (p.z > upper_bound.z) upper_bound.z = p.z;
                if (p.x < lower_bound.x) lower_bound.x = p.x;
                if (p.y < lower_bound.y) lower_bound.y = p.y;
                if (p.z < lower_bound.z) lower_bound.z = p.z;
            }
            // rescale resolution
            auto gap = upper_bound - lower_bound;
            auto n_grid_x = // resolution.x;
                    std::min(static_cast<int>(gap.x / scene.smooth_len + 1),
                             resolution.x);
            auto n_grid_y = // resolution.y;
                    std::min(static_cast<int>(gap.y / scene.smooth_len + 1),
                             resolution.y);
            auto n_grid_z = // resolution.z;
                    std::min(static_cast<int>(gap.z / scene.smooth_len + 1),
                             resolution.z);
            grid.resize(n_grid_x*n_grid_y*n_grid_z);
            // rescale cell size
            auto cell_x = // scene.smooth_len;
                std::max(scene.smooth_len, gap.x/n_grid_x);
            auto cell_y = // scene.smooth_len;
                std::max(scene.smooth_len, gap.y/n_grid_y);
            auto cell_z = // scene.smooth_len;
            std::max(scene.smooth_len, gap.z/n_grid_z);
            for (auto &g : grid) g.reserve(200);

            for (int i = 0; i < n_particles; i++)
            {
                auto gap = position[i] - lower_bound;
                auto x = std::min(n_grid_x - 1,
                                  static_cast<int>(gap.x / cell_x));
                auto y = std::min(n_grid_y - 1,
                                  static_cast<int>(gap.y / cell_y));
                auto z = std::min(n_grid_z - 1,
                                  static_cast<int>(gap.z / cell_z));
                auto index = (x * n_grid_y + y) * n_grid_z + z;
                grid[index].push_back(i);
                grid_index[i].x = x;
                grid_index[i].y = y;
                grid_index[i].z = z;
            }


#pragma omp parallel num_threads(8)
            {
                // density & pressure
#pragma omp for
                for (int i = 0; i < n_particles; i++)
                {
                    neighbors[i].clear();
                    auto rho = 0.f;
                    auto x_s = grid_index[i].x > 0 ? grid_index[i].x-1 : 0;
                    auto x_e = grid_index[i].x < n_grid_x -1 ? grid_index[i].x+2 : n_grid_x;
                    auto y_s = grid_index[i].y > 0 ? grid_index[i].y-1 : 0;
                    auto y_e = grid_index[i].y < n_grid_y -1 ? grid_index[i].y+2 : n_grid_y;
                    auto z_s = grid_index[i].z > 0 ? grid_index[i].z-1 : 0;
                    auto z_e = grid_index[i].z < n_grid_z -1 ? grid_index[i].z+2 : n_grid_z;

                    for (int ix = x_s; ix < x_e; ix++)
                    {
                        for (int iy = y_s; iy < y_e; iy++)
                        {
                            for (int iz = z_s; iz < z_e; iz++)
                            {
                                auto index =
                                        (ix * n_grid_y + iy) * n_grid_z + iz;
                                for (auto j: grid[index])
                                {
                                    auto r = glm::distance(position[i],
                                                           position[j]);
                                    if (r < scene.smooth_len)
                                    {
                                        auto r2 = r * r;
                                        auto rest_dist = scene.smooth_len - r;
                                        auto rest_dist2 = scene.smooth_len2 - r2;
                                        rho += mass_state[j]* rest_dist2 * rest_dist2 * rest_dist2;
                                        neighbors[i].emplace_back(j, r, r2, rest_dist, rest_dist2);
                                    }
                                }
                            }
                        }
                    }
                    density[i] = scene.K_poly6 * rho;
                    pressure[i] = std::max(0.f, scene.stiffness *
                                                (density[i] - rest_density_state[i]));
                }
                // force
#pragma omp for
                for (int i = 0; i < n_particles; i++)
                {
                    auto &r_i = position[i];
                    auto &v_i = velocity[i];
                    auto &p_i = pressure[i];
                    auto &rho_i = density[i];

                    auto f_p = glm::vec3(0.f);
                    auto f_v = glm::vec3(0.f);
                    auto delta_color = glm::vec3(0.f);
                    auto delta_color2 = 0.f;

                    for (auto &p : neighbors[i])
                    {
                        auto volume_rest = p.rest_dist * mass_state[p.index]/density[p.index];
                        if (p.dist != 0.f)
                        {
                            auto p_sum = p_i + pressure[p.index];
                            auto r = r_i - position[p.index];
                            auto normalized_r = r / p.dist;
                            if (p_sum != 0.f)
                                f_p += (p_sum*0.5f*volume_rest*p.rest_dist) * normalized_r;
                            delta_color += (volume_rest*p.rest_dist2*p.rest_dist2) * r;
                            delta_color2 += volume_rest*p.rest_dist2*(7.f*p.dist2 - 3.f*scene.smooth_len2);
                        }
                        f_v += (coef_viscosity_state[p.index]*volume_rest) * (velocity[p.index] - v_i);
                    }

                    acceleration[i] = glm::vec3(0.f);
                    if (scene.scene_param.pressure_force)
                        acceleration[i] += scene.K_delta_spiky*f_p;
                    if (scene.scene_param.viscosity_force)
                        acceleration[i] += scene.K_delta_spiky*f_v;
                    if (scene.scene_param.surface_tension)
                    {
                        auto len_delta_color = glm::length(delta_color);
                        if (len_delta_color != 0.f)
                            acceleration[i] += scene.neg_coef_surface_tension * scene.K_laplacian_poly6 *delta_color2 / len_delta_color * delta_color;
                    }
                    acceleration[i] /= rho_i;
                    if (scene.scene_param.gravity_force)
                        acceleration[i] += scene.scene_param.gravity;
                }
            }
        }

        void avoidCollision(float dt)
        {
            constexpr auto eps = 1e-6f;
            constexpr auto stiff = 5e4f;
            constexpr auto damp = 100.f;

#define BOUNDARY_CONSTRAINT(AXIS)                                               \
            {                                                                   \
                auto diff = scene.particle_diameter - (p.AXIS - lower_boundary.AXIS);\
                if (diff > eps) a.AXIS = stiff * diff - damp * v.AXIS;          \
            }                                                                   \
            {                                                                   \
                auto diff = (upper_boundary.AXIS - p.AXIS) - scene.particle_diameter;\
                if (diff < -eps) a.AXIS = stiff * diff - damp * v.AXIS;         \
            }

            for (int i = 0; i < n_particles; i++)
            {
                auto p = position[i] + velocity_h[i] * dt;
                auto &v = velocity[i];
                auto a = glm::vec3(0.f);


                // sphere
//                auto n = p - sphere.pos;
//                auto dist = glm::length(n);
//                auto diff = sphere.radius + scene.particle_diameter - dist;
//                while (dist == 0.f)
//                {
//                    n.x = rnd_np();
//                    n.y = rnd_np();
//                    n.z = rnd_np();
//                    dist = glm::length(n);
//                }
//                if (diff > eps)
//                {
//                    n /= dist;
//                    a += (stiff*diff - damp * glm::dot(v, n))*n;
//                }

                // cylinder
                for (auto &o : obstacles)
                {
                    auto n = p - glm::vec3(o.pos.x, p.y, o.pos.y);
                    auto dist = glm::length(n);
                    auto diff = o.radius + scene.particle_diameter - dist;
                    if (diff > eps)
                    {
                        if (p.y > o.bottom && p.y < o.top)
                        {
                            while (dist == 0.f)
                            {
                                n.x = rnd_np();
                                n.z = rnd_np();
                                dist = glm::length(n);
                            }
                            if (diff > eps)
                            {
                                n /= dist;
                                a += (stiff*diff - damp * glm::dot(v, n))*n;
                            }
                        }
                        else
                        {
                            diff = scene.particle_diameter - (p.y - o.top);
                            if (diff > eps) a.y = stiff * diff - damp * v.y;
//                            diff = (o.bottom - p.y) - scene.particle_diameter;
//                            if (diff < -eps) a.y = stiff * diff - damp * v.y;
                        }
                    }

                }

                BOUNDARY_CONSTRAINT(x);
                BOUNDARY_CONSTRAINT(y);
                BOUNDARY_CONSTRAINT(z);

                acceleration[i] += a;
            }
        }

        void collide()
        {
            constexpr auto damp = .8f; // resitiution
            constexpr auto eps = 1e-6f;

#define BOUNDARY_COLLIDE_HELPER(LOWER_or_UPPER, AXIS)                           \
            {                                                                   \
                auto t = (p.AXIS - LOWER_or_UPPER.AXIS)/v.AXIS;                 \
                p -= v*((1-damp)*t);                                            \
                p.AXIS = LOWER_or_UPPER.AXIS + LOWER_or_UPPER.AXIS - p.AXIS;    \
                v.AXIS = -v.AXIS; vh.AXIS = -vh.AXIS;                           \
                v *= damp; vh *= damp;                                          \
            }
#define BOUNDARY_COLLIDE(AXIS)                                                  \
            {                                                                   \
                if (p.AXIS < lower_boundary.AXIS && v.AXIS < -eps)        \
                    BOUNDARY_COLLIDE_HELPER(lower_boundary, AXIS)         \
                if (p.AXIS > upper_boundary.AXIS && v.AXIS > eps)         \
                    BOUNDARY_COLLIDE_HELPER(upper_boundary, AXIS)         \
            }


            struct Sphere
            {
                glm::vec3 pos = glm::vec3(0.f, 0.02f, 0.f);
                float radius = 0.02f;

            } sphere;

            for (int i = 0; i < n_particles; i++)
            {
                auto &p = position[i];
                auto &v = velocity[i];
                auto &vh = velocity_h[i];


                BOUNDARY_COLLIDE(x);
                BOUNDARY_COLLIDE(y);
                BOUNDARY_COLLIDE(z);
            }
        }

        void leapfrog(float dt)
        {
            auto n = n_particles+n_particles+n_particles;

            dynamics(dt);
            avoidCollision(dt);
            // leapfrog
            if (first_step == 0)
            {
                cblas_saxpy(n, // v = v(0) + a dt
                            dt, reinterpret_cast<float *>(acceleration.data()), 1,
                            reinterpret_cast<float *>(velocity.data()), 1);
                cblas_saxpy(n, // v_h = v(0) + a dt/2
                            dt*0.5f, reinterpret_cast<float *>(acceleration.data()), 1,
                            reinterpret_cast<float *>(velocity_h.data()), 1);
                first_step = false;
            }
            else
            {
                cblas_scopy(n,
                            reinterpret_cast<float *>(velocity_h.data()), 1,
                            reinterpret_cast<float *>(velocity.data()), 1);
                cblas_saxpy(n, // v = v_1/2 + a dt/2
                            dt*0.5f, reinterpret_cast<float *>(acceleration.data()), 1,
                            reinterpret_cast<float *>(velocity.data()), 1);
                cblas_saxpy(n, // v_1/2 = v_1/2 + a dt
                            dt, reinterpret_cast<float *>(acceleration.data()), 1,
                            reinterpret_cast<float *>(velocity_h.data()), 1);
            }
            cblas_saxpy(n, // p = p + v_1/2 dt
                        dt, reinterpret_cast<float *>(velocity_h.data()), 1,
                        reinterpret_cast<float *>(position.data()), 1);
//            collide();
        }

        void add(std::vector<glm::vec3> const &pos,
                 std::vector<glm::vec3> const &vel,
                 float mass = -1.f,
                 float rest_density = -1.f,
                 float coef_viscosity = -1.f)
        {
            auto n = static_cast<int>(pos.size());
            for (auto i = 0; i < n; i++)
            {
                mass_state.emplace_back(mass < 0.f ? scene.mass : mass);
                rest_density_state.emplace_back(rest_density < 0.f ? scene.rest_density : rest_density);
                coef_viscosity_state.emplace_back(coef_viscosity < 0.f ? scene.coef_viscosity : coef_viscosity);
            }
#if defined(USE_CUDA)
            if (use_cuda)
            {
                void *pos_buffer; size_t buffer_size;
                PX_CUDA_CHECK(cudaGraphicsMapResources(1, cuda_res, 0));
                PX_CUDA_CHECK(cudaGraphicsResourceGetMappedPointer(&pos_buffer, &buffer_size, cuda_res[0]));
                if (n_particles + n > static_cast<int>(position.capacity()))
                {
                    // TODO reallocate memeory
                }
                else
                {
                    scene.cudaAppend(reinterpret_cast<float3*>(pos_buffer), n_particles,
                                     reinterpret_cast<const float3*>(pos.data()), reinterpret_cast<const float3*>(vel.data()),
                                     mass_state.data()+n_particles,
                                     rest_density_state.data()+n_particles,
                                     coef_viscosity_state.data()+n_particles,
                                     n);
                    n_particles += n;
                    particle_shader.setInstances(n_particles);
                }
                PX_CUDA_CHECK(cudaDeviceSynchronize());
                PX_CUDA_CHECK(cudaGraphicsUnmapResources(1, cuda_res, 0));

            }
            else
            {
#endif
                for (auto &p : pos)
                {
                    n_particles++;
                    position.push_back(p);

                    acceleration.emplace_back(0.f);
                    density.emplace_back(0.f);
                    pressure.emplace_back(0.f);
                    grid_index.emplace_back(0.f);
                    neighbors.emplace_back(std::vector<Dist>());
                    neighbors.back().reserve(200);
                }
                for (auto &v : vel)
                {
                    velocity.push_back(v);
                    velocity_h.push_back(v);
                }
#if defined(USE_CUDA)
            }
#endif
        }

        void update(float dt, int n)
        {

#if defined(USE_CUDA)
            if (use_cuda)
            {
                void *pos_buffer; void *color_buffer; size_t buffer_size;
                PX_CUDA_CHECK(cudaGraphicsMapResources(2, cuda_res, 0));
                PX_CUDA_CHECK(cudaGraphicsResourceGetMappedPointer(&pos_buffer, &buffer_size, cuda_res[0]));
                PX_CUDA_CHECK(cudaGraphicsResourceGetMappedPointer(&color_buffer, &buffer_size, cuda_res[1]));

                auto pos = reinterpret_cast<float3*>(pos_buffer);
                auto color = reinterpret_cast<float*>(color_buffer);
                for (int i = 0; i < n; i++)
                    scene.cudaUpdate(pos, color, dt);

                PX_CUDA_CHECK(cudaDeviceSynchronize());
                PX_CUDA_CHECK(cudaGraphicsUnmapResources(2, cuda_res, 0));
            }
            else
#endif
            {
                for (int i = 0; i < n; i++)
                   leapfrog(dt);
                particle_shader.setInstances(position, n_particles);
            }
        }

        shader::ForwardPhong shader;
        std::shared_ptr<item::Ball> sphere;
        float gen_remaining;
    };

scene::SPHScene::SPHScene()
    : SPHScene(scene::SPHScene::SceneParameter(0.0008, 20, glm::vec3(0.f, -9.8f, 0.f)))
{}

scene::SPHScene::SPHScene(const scene::SPHScene::SceneParameter &scene_param)
    : scene_param(scene_param)
{
    pimpl = std::unique_ptr<impl>(new impl(*this));
}

scene::SPHScene::~SPHScene()
{}


   static unsigned int framebuffer;
   static unsigned int textureColorBufferMultiSampled;
   static unsigned int intermediateFBO;
   static unsigned int screenTexture;
   static int width = 1920;
   static int height = 1080;

void scene::SPHScene::init()
{

       glGenFramebuffers(1, &framebuffer);
       glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
       // create a multisampled color attachment texture
       glGenTextures(1, &textureColorBufferMultiSampled);
       glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
       glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 32, GL_RGB, width, height, GL_TRUE);
       glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
       glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
       // create a (also multisampled) renderbuffer object for depth and stencil attachments
       unsigned int rbo;
       glGenRenderbuffers(1, &rbo);
       glBindRenderbuffer(GL_RENDERBUFFER, rbo);
       glRenderbufferStorageMultisample(GL_RENDERBUFFER, 32, GL_DEPTH24_STENCIL8, width, height);
       glBindRenderbuffer(GL_RENDERBUFFER, 0);
       glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
       assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
       glBindFramebuffer(GL_FRAMEBUFFER, 0);
       // configure second post-processing framebuffer
       glGenFramebuffers(1, &intermediateFBO);
       glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
       // create a color attachment texture
       glGenTextures(1, &screenTexture);
       glBindTexture(GL_TEXTURE_2D, screenTexture);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);    // we only need a color buffer
       assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
       glBindFramebuffer(GL_FRAMEBUFFER, 0);

    pimpl->init();

    auto floor = std::make_shared<px::item::Floor>(10.f, 10.f);
    floor->scal(glm::vec3(1.f, 1.f, 1.f));
    floor->pos(glm::vec3(0.f));
    floor->init();
    addItem(floor);

//    auto ball = std::make_shared<px::item::TextureBall>();
//    ball->pos(glm::vec3(0.f, 0.02f, 0.f));
//    ball->scal(glm::vec3(0.02f));
//    ball->init();
//    addItem(ball);

    ForwardRenderingScene::init();

    restart();
}

auto capture = false;

void scene::SPHScene::update(float dt)
{
#ifndef USE_CUDA
    constexpr auto gen_rate = 20000;
#else
    auto gen_rate = pimpl->use_cuda ? 100000 : 20000;
#endif

    auto fr = 10;

    static int i = 0;
    // if (i == 0)
    // {
    //     capture = true;
    // }
    // else if (i % fr == 0)
    // {
    //     capture = true;
    // }
    // else
    // {
    //     capture = false;
    // }
    // if (i == 300)
    // {
    //     tap_open = true;
    // }
    // if (i == 450)
    // {
    //     camera()->position(glm::vec3(-.38307f, .329474f, .564863f));
    //     camera()->roll(0.f);
    //     camera()->pitch(20.5329f);
    //     camera()->yaw(34.0494f);
    // }
    // if (i == 650)
    // {
    //     camera()->position(glm::vec3(0.f, .25f, .75f));
    //     camera()->pitch(6.f);
    //     camera()->roll(0.f);
    //     camera()->yaw(0.f);
    // }
    // if (i == 700)
    // {
    //     tap_open = false;
    // }
    // if (i == 1500)
    // {
    //     tap_open = true;
    // }
    // if (i == 1700)
    // {
    //     tap_open = false;
    // }


    // if (i == 0)
    // {
    //     camera()->position(glm::vec3(-.38307f, .329474f, .564863f));
    //     camera()->roll(0.f);
    //     camera()->pitch(20.5329f);
    //     camera()->yaw(34.0494f);
    //     capture = true;
    // }
    // else if (i % fr == 0)
    // {
    //     capture = true;
    // }
    // else
    // {
    //     capture = false;
    // }
    // if (i == 30)
    // {
    //     tap_open = true;
    // }
    // if (i == 200)
    // {
    //     tap_open = false;
    // }
    // if (i == 450)
    // {
    //     camera()->position(glm::vec3(0.f, .25f, .75f));
    //     camera()->pitch(6.f);
    //     camera()->roll(0.f);
    //     camera()->yaw(0.f);
    // }

    // if (i == 0)
    // {
    //     camera()->position(glm::vec3(0.f, .2f, .55f));
    //     camera()->pitch(6.f);
    //     camera()->roll(0.f);
    //     camera()->yaw(0.f);
    //     capture = true;
    // }
    // else if (i % fr == 0)
    // {
    //     capture = true;
    // }
    // else
    // {
    //     capture = false;
    // }
    // if (i == 150)
    // {
    //     camera()->position(glm::vec3(0.f, .6f, 0.f));
    //     camera()->roll(0.f);
    //     camera()->pitch(90.0f);
    //     camera()->yaw(0.f);
    // }
    // if (i == 300)
    // {
    //     tap_open = true;
    // }
    // if (i == 550)
    // {
    //     tap_open = false;
    // }
    // if (i == 700)
    // {
    //     camera()->position(glm::vec3(0.f, .2f, .55f));
    //     camera()->pitch(6.f);
    //     camera()->roll(0.f);
    //     camera()->yaw(0.f);
    // }


    if (i == 0)
    {
        camera()->position(glm::vec3(0.f, .35f, .75f));
        camera()->pitch(6.f);
        camera()->roll(0.f);
        camera()->yaw(0.f);
        show_box = false;
        capture = true;
    }
    else if (i % fr == 0)
    {
        capture = true;
    }
    else
    {
        capture = false;
    }
    if (i == 20)
    {
        camera()->position(glm::vec3(0.f, 1.05f, 0.f));
        camera()->pitch(90.f);
        camera()->roll(0.f);
        camera()->yaw(0.f);
    }
    if (i == 40)
    {
        camera()->position(glm::vec3(0.f, .35f, .75f));
        camera()->pitch(6.f);
        camera()->roll(0.f);
        camera()->yaw(0.f);
    }
    if (i == 240 || i == 640)
    {
        camera()->position(glm::vec3(0.f, 1.05f, 0.f));
        camera()->pitch(90.f);
        camera()->roll(0.f);
        camera()->yaw(0.f);
    }
    if (i == 440 || i == 840)
    {
        camera()->position(glm::vec3(0.f, .35f, .75f));
        camera()->pitch(6.f);
        camera()->roll(0.f);
        camera()->yaw(0.f);
    }



    i++;

    if (dt > 0.f)
    {
        // auto n = std::min(scene_param.max_steps, static_cast<int>(std::ceil(dt / scene_param.step_size)));
        auto n = 6;
        if (tap_open)
        {
            if (particles() < static_cast<int>(pimpl->position.capacity()))
            {
                auto gen_new = pimpl->gen_remaining + gen_rate * scene_param.step_size*n;
                auto gn = std::min(pimpl->position.capacity()-particles(), static_cast<std::size_t>(gen_new));

                std::vector<glm::vec3> pos;
                std::vector<glm::vec3> vel;
                pos.reserve(gn);
                vel.reserve(gn);
                for (auto i = gn; i >= 1; i--)
                {
                    pos.emplace_back(rnd_np()*particle_diameter*1.75f+pimpl->upper_boundary.x-particle_diameter*2.f,
                                     rnd_np()*smooth_len*.5f+pimpl->upper_boundary.y*0.75f,
                                     rnd_np()*smooth_len*.5f);
                    vel.emplace_back(-5.5f, -2.f, 0.f);
                }
                pimpl->add(pos, vel);
                pimpl->gen_remaining = gen_new - gn;
            }
        }
        pimpl->update(scene_param.step_size, n);
    }
    ForwardRenderingScene::update(scene_param.step_size);
}

void scene::SPHScene::restart()
{
    pimpl->restart();
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_writer.h"
#include <iomanip>
#include <sstream>

void scene::SPHScene::render()
{

    if (capture)
    {
       glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
       glClearColor(0.f, 0.f, 0.f, 1.f);
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (show_box)
        pimpl->box_shader.render();
    if (show_obs)
        pimpl->obstacle_shader.render();
    pimpl->particle_shader.render(scenery != Scenery::Empty);
    ForwardRenderingScene::render();
    
    
    if (capture)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
        std::vector<unsigned char> pixels(3*width*height);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0,width, height,GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


        std::vector<unsigned char> reordered(3*width*height);
        for (auto x = 0; x < width; x++)
        {
            for (auto y = 0; y < height; y++)
            {
                reordered.at((y*width+x)*3) = pixels.at(((height-y-1)*width+x)*3);
                reordered.at((y*width+x)*3+1) = pixels.at(((height-y-1)*width+x)*3+1);
                reordered.at((y*width+x)*3+2) = pixels.at(((height-y-1)*width+x)*3+2);
            }
        }

        static int i = 162;
        i++;
        std::ostringstream ss;
        ss << "frame" << std::setfill('0') << std::setw(3) << i << ".png";
        stbi_write_png(ss.str().data(),
                        width, height, 3, reordered.data(), width*3);
        if (i == 300) exit(0);
    }
}

int scene::SPHScene::particles()
{
    return pimpl->n_particles;
}

bool scene::SPHScene::isUseCuda()
{
    return pimpl->use_cuda;
}
