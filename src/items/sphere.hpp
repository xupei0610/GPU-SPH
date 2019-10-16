#ifndef PX_CG_ITEMS_SPHERE_HPP
#define PX_CG_ITEMS_SPHERE_HPP

#include "item.hpp"
namespace px { namespace item {
    class Sphere;
}}

class px::item::Sphere : public px::Item
{
public:

    Sphere();
    ~Sphere() override = default;

    float mass() override;
    void mass(float m) override;
    glm::vec3 pos() override;
    void pos(glm::vec3 const &p) override;
    glm::vec3 scal() override;
    void scal(glm::vec3 const &s) override;
    glm::vec3 rot() override;
    void rot(glm::vec3 const &r) override;
    glm::vec3 vel() override;
    void vel(glm::vec3 const &v) override;
    void force(glm::vec3 const &f) override;
    glm::vec3 force() override;
    void resistance(glm::vec3 const &r) override;
    glm::vec3 resistance() override;

    void update(float dt) override;

protected:
    inline glm::mat4 &model() {return model_;}
    void updateModel();

private:
    float mass_;
    glm::vec3 pos_;
    glm::vec3 scal_;
    glm::vec3 rot_;
    glm::vec3 vel_;
    glm::mat4 model_;
    glm::vec3 force_;
    glm::vec3 resistance_;
};

#endif
