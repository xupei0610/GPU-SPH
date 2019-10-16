#include "deferred_rendering_scene.hpp"

using namespace px;

scene::DeferredRenderingScene::DeferredRenderingScene()
    : Scene()
{}

void scene::DeferredRenderingScene::init()
{
    Scene::init();

    pass_shader.init();
    lighting_shader.init();

    pass_shader.activate(true);
    pass_shader.set("global_ambient", glm::vec3(.5f, .5f, .5f));
    pass_shader.activate(false);
}

void scene::DeferredRenderingScene::resize(unsigned int width, unsigned int height)
{
    Scene::resize(width, height);

    pass_shader.resize(width, height);
    lighting_shader.resize(width, height);
}

void scene::DeferredRenderingScene::render()
{
    GLint draw_fbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_fbo);

    pass_shader.preActivate(true);
    pass_shader.activate(true);
    // object rendering
    for (auto &o: items()) o->render(pass_shader);
    pass_shader.preActivate(false);
    pass_shader.activate(false);

    lighting_shader.activate(true);
    auto counter = 0;
    for (auto &o : items())
    {
        if (o->light())
        {
            // setup lights
            lighting_shader.set("lights[" + std::to_string(counter) + "].position", o->pos());
            lighting_shader.set("lights[" + std::to_string(counter) + "].ambient",  o->ambient());
            lighting_shader.set("lights[" + std::to_string(counter) + "].diffuse",  o->diffuse());
            lighting_shader.set("lights[" + std::to_string(counter) + "].specular", o->specular());
            lighting_shader.set("lights[" + std::to_string(counter) + "].coef",     o->lightAttenuation());

            ++counter;
            if (counter == shader::DeferredLighting::MAX_LIGHTS_PER_BATCH)
            {
                lighting_shader.renderCache(counter, pass_shader);
                counter = 0;
            }
        }
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_fbo);
    lighting_shader.render(counter, pass_shader);
    lighting_shader.activate(false);

    pass_shader.extractDepthBuffer();

    Scene::render();
}