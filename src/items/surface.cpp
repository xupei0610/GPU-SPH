#include "surface.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace px;

item::Surface::Surface()
    : Item(), scal_(1.f), rot_(0.f), pos_(0.f),
      norm_(0.f, 1.f, 0.f), tangent_x_(1.f,0.f,0.f), tangent_z_(0.f,0.f,1.f),
      friction_(0.f), restitution_(1.f)
{
    updateModelMat();
}

glm::vec3 item::Surface::scal()
{
    return scal_;
}

void item::Surface::scal(glm::vec3 const &s)
{
    scal_ = s;
    updateModelMat();
}

glm::vec3 item::Surface::rot()
{
    return rot_;
}

void item::Surface::rot(glm::vec3 const &r)
{
    rot_ = r;
    updateModelMat();
    norm_ = rot_mat_ * glm::vec4(0.f, 1.f, 0.f, 0.f);
    tangent_x_ = rot_mat_ * glm::vec4(1.f, 0.f, 0.f, 0.f);
    tangent_z_ = rot_mat_ * glm::vec4(0.f, 0.f, 1.f, 0.f);
}

glm::vec3 item::Surface::pos()
{
    return pos_;
}

void item::Surface::pos(glm::vec3 const &p)
{
    pos_ = p;
    updateModelMat();
}

void item::Surface::updateModelMat()
{
    rot_mat_ = glm::eulerAngleYXZ(GLM_ANG(rot_.y), GLM_ANG(rot_.x), GLM_ANG(rot_.z));
    model_ = glm::translate(Item::IDENTITY_MAT, pos())*glm::scale(rot_mat_, scal());
}
