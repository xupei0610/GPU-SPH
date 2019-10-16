#ifndef PX_CG_SCENES_DEFERRED_RENDERING_SCENE_HPP
#define PX_CG_SCENES_DEFERRED_RENDERING_SCENE_HPP

#include "scene.hpp"
#include "shaders/deferred_lighting.hpp"

namespace px { namespace scene {
    class DeferredRenderingScene;
} }


class px::scene::DeferredRenderingScene : public Scene
{
public:
    DeferredRenderingScene();
    ~DeferredRenderingScene() override = default;

    void init() override;
    void render() override;
    void resize(unsigned int width, unsigned int height) override;

protected:
    shader::DeferredLightingPass pass_shader;
    shader::DeferredLighting lighting_shader;
};

#endif
