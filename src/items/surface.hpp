#ifndef PX_CG_ITEMS_PLANE_HPP
#define PX_CG_ITEMS_PLANE_HPP

#include "item.hpp"

namespace px { namespace item {
    class Surface;
}}

class px::item::Surface : public Item
{
public:
    Surface();
    ~Surface() override = default;

    glm::vec3 scal() override;
    void scal(glm::vec3 const &s) override;
    glm::vec3 rot() override;
    void rot(glm::vec3 const &r) override;

    glm::vec3 pos() override;
    void pos(glm::vec3 const &p) override;

    virtual void friction(float friction) {friction_ = friction;}
    virtual float friction() {return friction_; }
    inline glm::vec3 const& norm() { return norm_; }
    inline glm::vec3 const& tangentX() { return tangent_x_; }
    inline glm::vec3 const& tangentZ() { return tangent_z_; }

    CollisionShape collision() override { return CollisionShape::Plane; }
    virtual void restitution(float c) {restitution_ = c;}
    virtual float restitution() { return restitution_;}
    
protected:
    unsigned int vao, vbo, texture[4];
    void updateModelMat();
    inline glm::mat4 const &model() const noexcept {return model_;}

private:
    glm::vec3 scal_;
    glm::vec3 rot_;
    glm::vec3 pos_;
    glm::mat4 model_;
    glm::vec3 norm_;
    glm::vec3 tangent_x_;
    glm::vec3 tangent_z_;
    glm::mat4 rot_mat_;
    float friction_;
    float restitution_;
};
#endif
