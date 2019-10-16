#include "rigid_body.hpp"

using namespace px;

item::RigidBody::RigidBody()
{}

item::RigidBody::~RigidBody()
{}

void item::RigidBody::applyTorque(glm::vec3 const &local_position,
                                  glm::vec3 const &force)
{
    auto R = glm::mat3_cast(orient());
    q += glm::cross(R * local_position, force);
    f += force;
}

void item::RigidBody::clearTorque()
{
    q = glm::vec3(0.f);
    f = glm::vec3(0.f);
}

void item::RigidBody::step(float dt)
{
    auto a = f / mass() + (*g);
    R = glm::mat3_cast(orient());
    I_inv_t = R * I_inv * glm::transpose(R);
    angularVel(angularVel() + I_inv_t * (dt*q));
    vel(vel() + a * dt);
}

void item::RigidBody::apply(float dt)
{
    pos(pos() + vel() * dt);

    auto dw = dt*0.5f*angularVel();

    orient(glm::normalize(glm::quat(orient() + glm::quat(0, dw.x, dw.y, dw.z)*orient())));
    R = glm::mat3_cast(orient());
}
