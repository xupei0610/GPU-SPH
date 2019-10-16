#include "scene.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace px;

Scene::Scene()
    : camera_param_ubo(0)
{}

Scene::~Scene()
{
    glDeleteBuffers(1, &camera_param_ubo);
}

void Scene::init()
{
    glDeleteBuffers(1, &camera_param_ubo);
    camera_param_ubo = 0;

    glGenBuffers(1, &camera_param_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, camera_param_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*2+sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, camera_param_ubo, 0, sizeof(glm::mat4)*2+sizeof(glm::vec3));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (auto &o : items())
        o->init();
}

void Scene::update(float dt)
{
    if (camera() != nullptr)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, camera_param_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER,                   0, sizeof(glm::mat4),
                        glm::value_ptr(camera()->view()));
        glBufferSubData(GL_UNIFORM_BUFFER,   sizeof(glm::mat4), sizeof(glm::mat4),
                        glm::value_ptr(camera()->projection()));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*2, sizeof(glm::vec3),
                        glm::value_ptr(camera()->position()));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    for (auto &o : items()) o->update(dt);
}

void Scene::resize(unsigned int width, unsigned int height)
{
    width = 1920;
    height = 1080;
    if (camera() == nullptr)
    {
        glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    }
    else
    {
        camera()->filmSize(width, height);
        glViewport(0, 0, static_cast<int>(camera()->filmWidth()), static_cast<int>(camera()->filmHeight()));
    }
}

void Scene::render()
{
    for (auto &o: items())
        o->post_render();
}

std::size_t Scene::addItem(std::shared_ptr<Item> const &item)
{
    items_.push_back(item);
    return items_.size() - 1;
}