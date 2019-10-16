#ifndef PX_CG_ITEMS_RIGID_BODY
#define PX_CG_ITEMS_RIGID_BODY

#include <iostream>

#include "item.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace px {namespace item {
class RigidBody;
}}

class px::item::RigidBody : public px::Item
{
public:
    RigidBody();
    ~RigidBody();

    inline void setVisualShape(std::shared_ptr<px::Item> v)
    {
        this->visual_shape = std::move(v);
    }

    inline void setMomentOfInertia(glm::mat3 const &I)
    {
        this->I_inv = glm::inverse(I);
    }

    inline void bindPositionProperty(glm::vec3 *x)
    {
        this->x_ptr = x;
    }
    inline void bindMassProperty(float *m)
    {
        this->m_ptr = m;
    }
    inline void bindLinearVelocityProperty(glm::vec3 *v)
    {
        this->v_ptr = v;
    }
    inline void bindOrientationProperty(glm::quat *r)
    {
        this->r_ptr = r;
    }
    inline void bindAngularVelocityProperty(glm::vec3 *w)
    {
        this->w_ptr = w;
    }

    void bindGravityProperty(glm::vec3 *g)
    {
        this->g = g;
    }

    float mass() override { return m_ptr == nullptr ? m : *m_ptr;; }
    glm::vec3 pos() override { return x_ptr == nullptr ? x : *x_ptr; }
    glm::vec3 vel() override { return v_ptr == nullptr ? v : *v_ptr; }
    glm::vec3 force() override { return f; }
    glm::vec3 rot() override {
        return glm::eulerAngles(orient());
    }
    inline glm::quat orient() { return r_ptr == nullptr ? r : *r_ptr; }
    inline glm::vec3 angularVel() { return w_ptr == nullptr ? this->w : *w_ptr;}

    void mass(float m) override
    {
        if (m_ptr == nullptr) this->m = m;
        else *m_ptr = m;
    }
    void pos(glm::vec3 const &x) override
    {
        if (v_ptr == nullptr) this->x = x;
        else *x_ptr = x;
    }
    void vel(glm::vec3 const &v) override
    {
        if (v_ptr == nullptr) this->v = v;
        else *v_ptr = v;
    }
    void force(glm::vec3 const &) override
    {
        error("Not support to apply force on a rigid body explicitly.");
    }
    void rot(glm::vec3 const &r) override
    {
        if (r_ptr == nullptr) this->r = glm::quat(r);
        else *r_ptr = glm::quat(r);
    }
    inline void orient(glm::quat const &r)
    {
        if (r_ptr == nullptr) this->r = r;
        else *r_ptr = r;
    }
    inline void angularVel(glm::vec3 const &w)
    {
        if (w_ptr == nullptr) this->w = w;
        else *w_ptr = w;
    }

//    void resistance(glm::vec3 const &) override;
//    glm::vec3 resistance(glm::vec3) override;
//
//    void update(float dt) override
//    {
//        visual_shape->update(dt);
//    };
//    void init() override { visual_shape->init();}



    void applyTorque(glm::vec3 const &local_position,
                     glm::vec3 const &force);
    virtual void step(float dt);
    virtual void contact(std::vector<std::shared_ptr<RigidBody>> const &objs) = 0;
    virtual void constraint(std::vector<std::shared_ptr<RigidBody>> const &objs) = 0;
    virtual void apply(float dt);
    void clearTorque();

public:
    void render(Shader &shader) override
    {
        visual_shape->pos(pos());
        visual_shape->rot(rot());
        visual_shape->render(shader);
    }

    bool light() override { return visual_shape->light(); }
    glm::vec3 ambient() override { return visual_shape->ambient(); }
    glm::vec3 diffuse() override { return visual_shape->diffuse(); }
    glm::vec3 specular() override { return visual_shape->specular(); }
    glm::vec3 lightAttenuation() override { return visual_shape->lightAttenuation(); }
    void ambient(glm::vec3 const &c) override { return visual_shape->ambient(c); }
    void diffuse(glm::vec3 const &c) override { return visual_shape->diffuse(c); }
    void specular(glm::vec3 const &c) override { return visual_shape->specular(c); }
    void lightAttenuation(glm::vec3 const &c) override { return visual_shape->lightAttenuation(c); }
    void post_render() override { return visual_shape->post_render(); }

protected:
    std::shared_ptr<px::Item> visual_shape;
    glm::mat3 I_inv;
    float *m_ptr = nullptr;
    glm::vec3 *x_ptr = nullptr;
    glm::vec3 *v_ptr = nullptr;
    glm::quat *r_ptr = nullptr;
    glm::vec3 *w_ptr = nullptr;
    glm::vec3 *g = nullptr;

    glm::vec3 q = glm::vec3(0.f);
    glm::vec3 f = glm::vec3(0.f);

    float m = 0.f;
    glm::vec3 x = glm::vec3(0.f);
    glm::vec3 v = glm::vec3(0.f);
    glm::quat r = glm::quat(1.f, 0.f, 0.f, 0.f);
    glm::vec3 w = glm::vec3(0.f);
    glm::mat3 I_inv_t = glm::mat3(1.f);
    glm::mat3 R = glm::mat3(1.f);

};


#endif //PX_CG_ITEMS_RIGID_BODY
