#ifndef PX_CG_ITEMS_RIGID_PLANE_HPP
#define PX_CG_ITEMS_RIGID_PLANE_HPP

#include "items/rigid_body.hpp"

namespace px { namespace item { namespace rigid {
class Plane;
}}}

class px::item::rigid::Plane : public px::item::RigidBody
{
public:
    Plane();
    explicit Plane(float r);
    explicit Plane(glm::vec3 const & r);
    ~Plane() override;

    void scal(glm::vec3 const &s) override;
    glm::vec3 scal() override;

    void step(float dt) override;
    void update(float dt) override;
    void constraint(std::vector<std::shared_ptr<px::item::RigidBody>> const &objs) override;
    void contact(std::vector<std::shared_ptr<px::item::RigidBody>> const &objs) override;
    void apply(float dt) override;

    CollisionShape collision() override;
    std::pair<glm::vec3, glm::vec3> AABB() { return aabb_; }

protected:
    glm::vec3 scal_;
    std::pair<glm::vec3, glm::vec3> aabb_;
    static const std::array<glm::vec3, 4> vertices;
};

#endif //PX_CG_ITEMS_RIGID_SPHERE_HPP
