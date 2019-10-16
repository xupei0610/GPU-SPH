#ifndef PX_CG_ITEM_HPP
#define PX_CG_ITEM_HPP

#include <glm.hpp>
#include <memory>

#include "shader.hpp"

namespace px {
namespace item {
struct Material
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

enum class CollisionShape : int
{
    None = 1,
    Plane = 2,
    Sphere = 3,
    Cube = 4
};

}
class Item;
}

class px::Item
{
public:
    static const glm::mat4 IDENTITY_MAT;
    static const glm::vec3 X_AXIS;
    static const glm::vec3 Y_AXIS;
    static const glm::vec3 Z_AXIS;

    inline
    static bool intersect(std::pair<glm::vec3, glm::vec3> const &lhs,
                          std::pair<glm::vec3, glm::vec3> const &rhs)
    {
        return (lhs.second.x >= rhs.first.x && lhs.first.x <= rhs.second.x) &&
               (lhs.second.y >= rhs.first.y && lhs.first.y <= rhs.second.y) &&
               (lhs.second.z >= rhs.first.z && lhs.first.z <= rhs.second.z);
    }
public:
    Item();
    virtual ~Item() = default;
    [[noreturn]]
    virtual void error(std::string const &msg);
    [[noreturn]]
    virtual void error(std::string const &msg, int code);

    virtual glm::vec3 scal() { return glm::vec3(1.f); }
    virtual void scal(glm::vec3 const &s) {}
    virtual glm::vec3 rot() {return glm::vec3(0.f); }
    virtual void rot(glm::vec3 const &r) {}

    virtual float mass() { return 0.f; }
    virtual void mass(float m) {}

    virtual glm::vec3 pos() { return glm::vec3(0.f); }
    virtual void pos(glm::vec3 const &p) {}

    virtual glm::vec3 vel() { return glm::vec3(0.f); }
    virtual void vel(glm::vec3 const &v) {}

    virtual void force(glm::vec3 const &f) {}
    virtual glm::vec3 force() { return glm::vec3(0.f); }
    virtual void resistance(glm::vec3 const &r) {}
    virtual glm::vec3 resistance() { return glm::vec3(0.f); }
    
    virtual void init() {}
    virtual void update(float dt) {}

    virtual void render(Shader &shader) {}
    virtual bool light() { return false; }
    virtual glm::vec3 ambient() { return glm::vec3(0.f); }
    virtual void ambient(glm::vec3 const &c) {}
    virtual glm::vec3 diffuse() { return glm::vec3(0.f); }
    virtual void diffuse(glm::vec3 const &c) {}
    virtual glm::vec3 specular() { return glm::vec3(0.f); }
    virtual void specular(glm::vec3 const &c) {}
    virtual glm::vec3 lightAttenuation() { return glm::vec3(1.f); }
    virtual void lightAttenuation(glm::vec3 const &a) {}
    virtual void post_render() {}

    virtual item::CollisionShape collision() { return item::CollisionShape::None; }
    virtual std::pair<glm::vec3, glm::vec3> AABB() { return {glm::vec3(0.f), glm::vec3(0.f)};}
};

#endif 