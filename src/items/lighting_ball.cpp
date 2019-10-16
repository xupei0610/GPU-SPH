#include "lighting_ball.hpp"
#include "utils/shape_generator.hpp"
#include "utils/random.hpp"

using namespace px;

item::LightingBall::LightingBall()
    : Sphere()
{}

glm::vec3 item::LightingBall::color()
{
    return color_;
}

void item::LightingBall::color(glm::vec3 const &c)
{
    color_ = c;
    ambient(color_*0.f);
    diffuse(color_*1.2f);
    specular(color_);
}

glm::vec3 item::LightingBall::ambient()
{
    return ambient_;
}

void item::LightingBall::ambient(glm::vec3 const &c)
{
    ambient_ = c;
}

glm::vec3 item::LightingBall::diffuse()
{
    return diffuse_;
}

void item::LightingBall::diffuse(glm::vec3 const &c)
{
    diffuse_ = c;
}

glm::vec3 item::LightingBall::specular()
{
    return specular_;
}

void item::LightingBall::specular(glm::vec3 const &c)
{
    specular_ = c;
}

glm::vec3 item::LightingBall::lightAttenuation()
{
    return attenuation_;
}

void item::LightingBall::lightAttenuation(glm::vec3 const &a)
{
    attenuation_ = a;
}


item::LightingBallCollection::LightingBallCollection()
    : Item(), radius(1.f)
{}

void item::LightingBallCollection::LightingBallCollection::init()
{
    auto sphere = generator::sphere(12, radius);
    shader.init();
    shader.setVertices(sphere.first.data(), sphere.first.size(),
                       sphere.second.data(), sphere.second.size());
    update_position_ = true;
    update_color_ = true;
}

void item::LightingBallCollection::position(std::size_t id, glm::vec3 const &p)
{
    position_.at(id) = p;
    update_position_ = true;
}

void item::LightingBallCollection::color(std::size_t id, glm::vec3 const &c)
{
    color_.at(id) = c;
    update_color_ = true;
}


void item::LightingBallCollection::LightingBallCollection::post_render()
{
    if (updatePosition() || updateColor())
    {
        if (updatePosition())
        {
            if (updateColor())
                shader.setInstances(reinterpret_cast<const float*>(position().data()),
                                    reinterpret_cast<const float*>(color().data()),
                                    size());
            else
                shader.setInstances(reinterpret_cast<const float*>(position().data()),
                                    nullptr, size());
        }
        else 
            shader.setInstances(nullptr,
                                reinterpret_cast<const float*>(color().data()),
                                size());
        update_position_ = false;
        update_color_ = false;
    }

    shader.activate(true);
    shader.render(GL_TRIANGLES);
    shader.activate(false);
}

std::size_t item::LightingBallCollection::LightingBallCollection::add(std::shared_ptr<LightingBall> b)
{
    balls_.push_back(b);
    position_.emplace_back(b->pos());
    color_.emplace_back(b->color());
    update_position_ = true;
    update_color_ = true;
    return size() - 1;
}
