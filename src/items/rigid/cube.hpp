#ifndef PX_CG_ITEMS_RIGID_CUBE_HPP
#define PX_CG_ITEMS_RIGID_CUBE_HPP

#include "items/rigid_body.hpp"

namespace px { namespace item { namespace rigid {
class Cube;
}}}

class px::item::rigid::Cube : public px::item::RigidBody
{
public:
    static const std::array<glm::vec3, 8> vertices;

    Cube();
    explicit Cube(float s);
    explicit Cube(glm::vec3 const & s);
    ~Cube() override;

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
    std::vector<glm::vec3> vertices_;
};

#endif //PX_CG_ITEMS_RIGID_Cube_HPP
