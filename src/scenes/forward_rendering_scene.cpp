#include "forward_rendering_scene.hpp"

using namespace px;

scene::ForwardRenderingScene::ForwardRenderingScene()
        : Scene()
{}

void scene::ForwardRenderingScene::init()
{
    Scene::init();

    forward_shader.init();

    forward_shader.activate(true);
    forward_shader.set("global_ambient", glm::vec3(.5f, .5f, .5f));
    forward_shader.activate(false);
}


void scene::ForwardRenderingScene::render()
{
    forward_shader.activate(true);
    auto counter = 0;
    for (auto &o : items())
    {
        if (o->light())
        {
            forward_shader.set("lights[" + std::to_string(counter) + "].position", o->pos());
            forward_shader.set("lights[" + std::to_string(counter) + "].ambient",  o->ambient());
            forward_shader.set("lights[" + std::to_string(counter) + "].diffuse",  o->diffuse());
            forward_shader.set("lights[" + std::to_string(counter) + "].specular", o->specular());
            forward_shader.set("lights[" + std::to_string(counter) + "].coef",     o->lightAttenuation());

            ++counter;
        }
        if (counter >= shader::ForwardPhongTexture::MAX_LIGHTS)
            break;
    }
    forward_shader.set("n_lights", counter);

    for (auto &o: items()) o->render(forward_shader);

    forward_shader.activate(false);

    Scene::render();
}
