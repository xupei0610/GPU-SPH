#ifndef PX_CG_ITEMS_RIGID_SPHERE_HPP
#define PX_CG_ITEMS_RIGID_SPHERE_HPP

#include "items/rigid_body.hpp"

namespace px { namespace item { namespace rigid {
class Sphere;
}}}

class px::item::rigid::Sphere : public px::item::RigidBody
{
public:
    Sphere();
    explicit Sphere(float r);
    explicit Sphere(glm::vec3 const & r);
    ~Sphere() override;

    void scal(glm::vec3 const &s);
    glm::vec3 scal() override;

    void update(float dt) override;
    void constraint(std::vector<std::shared_ptr<px::item::RigidBody>> const &objs) override;
    void contact(std::vector<std::shared_ptr<px::item::RigidBody>> const &objs) override;

    CollisionShape collision() override;
    std::pair<glm::vec3, glm::vec3> AABB() { return aabb_; }

protected:
    glm::vec3 scal_;
    std::pair<glm::vec3, glm::vec3> aabb_;
};

#endif //PX_CG_ITEMS_RIGID_SPHERE_HPP
