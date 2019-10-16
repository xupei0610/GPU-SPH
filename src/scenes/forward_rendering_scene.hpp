#ifndef PX_CG_SCENES_FORWARD_RENDERING_HPP
#define PX_CG_SCENES_FORWARD_RENDERING_HPP

#include "scene.hpp"
#include "shaders/forward_phong_texture.hpp"

namespace px { namespace scene {
    class ForwardRenderingScene;
}}

class px::scene::ForwardRenderingScene : public Scene
{
public:
    ForwardRenderingScene();
    ~ForwardRenderingScene() override = default;

    void init() override;
    void render() override;

protected:
    shader::ForwardPhongTexture forward_shader;
};

#endif
