#include "plane.hpp"

using namespace px;

const std::array<glm::vec3, 4> item::rigid::Plane::vertices = {
    glm::vec3(-1.f, 0.f, -1.f), glm::vec3(-1.f, 0.f, 1.f),
    glm::vec3(1.f, 0.f, 1.f), glm::vec3(1.f, 0.f, -1.f)
};

item::rigid::Plane::Plane()
    : Plane(glm::vec3(std::numeric_limits<float>::infinity()))
{}

item::rigid::Plane::Plane(float s)
        : Plane(glm::vec3(s))
{}

item::rigid::Plane::Plane(glm::vec3 const &s)
        : RigidBody()
{
    scal(s);
}

item::rigid::Plane::~Plane()
{}

void item::rigid::Plane::scal(glm::vec3 const &s)
{
    scal_ = s;
    scal_.y = 0.f;
}

glm::vec3 item::rigid::Plane::scal()
{
    return scal_;
}

item::CollisionShape item::rigid::Plane::collision()
{
    return CollisionShape::Plane;
}

void item::rigid::Plane::update(float dt)
{
    auto model = glm::mat3(glm::scale(glm::mat4_cast(orient()), scal_));
    aabb_.first.x = std::numeric_limits<float>::max();
    aabb_.first.y = std::numeric_limits<float>::max();
    aabb_.first.z = std::numeric_limits<float>::max();
    aabb_.second.x = std::numeric_limits<float>::min();
    aabb_.second.y = std::numeric_limits<float>::min();
    aabb_.second.z = std::numeric_limits<float>::min();
    for (auto const &v: vertices)
    {
        auto real_v = model * v + pos();
        aabb_.first.x = std::min(aabb_.first.x, real_v.x);
        aabb_.first.y = std::min(aabb_.first.y, real_v.x);
        aabb_.first.z = std::min(aabb_.first.z, real_v.x);
        aabb_.second.x = std::max(aabb_.second.x, real_v.x);
        aabb_.second.y = std::max(aabb_.second.y, real_v.x);
        aabb_.second.z = std::max(aabb_.second.z, real_v.x);
    }
}

void item::rigid::Plane::step(float dt)
{}

void item::rigid::Plane::constraint(std::vector<std::shared_ptr<px::item::RigidBody>> const &objs)
{}

void item::rigid::Plane::contact(std::vector<std::shared_ptr<px::item::RigidBody>> const &objs)
{}

void item::rigid::Plane::apply(float dt)
{}
