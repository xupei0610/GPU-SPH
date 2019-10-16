#include "sphere.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace px;

item::Sphere::Sphere()
    : Item(), pos_(0.f), scal_(1.f), rot_(0.f), vel_(0.f), force_(0.f), resistance_(0.f)
{
    updateModel();
}

float item::Sphere::mass()
{
    return mass_;
}

void item::Sphere::mass(float m)
{
    mass_ = m;
}

glm::vec3 item::Sphere::pos()
{
    return pos_;
}

void item::Sphere::pos(glm::vec3 const &p)
{
    pos_ = p;
    updateModel();
}

glm::vec3 item::Sphere::scal()
{
    return scal_;
}

void item::Sphere::scal(glm::vec3 const &s)
{
    scal_ = s;
    updateModel();
}

glm::vec3 item::Sphere::rot()
{
    return rot_;
}

void item::Sphere::rot(glm::vec3 const &r)
{
    rot_ = r;
    updateModel();
}

glm::vec3 item::Sphere::vel()
{
    return vel_;
}

void item::Sphere::vel(glm::vec3 const &v)
{
    vel_ = v;
}

void item::Sphere::force(glm::vec3 const &f)
{
    force_ = f;
}

glm::vec3 item::Sphere::force()
{
    return force_;
}

void item::Sphere::resistance(glm::vec3 const &r)
{
    resistance_ = r;
}

glm::vec3 item::Sphere::resistance()
{
    return resistance_;
}

void item::Sphere::updateModel()
{
    model_ = glm::translate(IDENTITY_MAT, pos()) *
             glm::scale(glm::eulerAngleYXZ(rot_.y, rot_.x, rot_.z), scal());
}

void item::Sphere::update(float dt)
{
//    constexpr auto edge_eps = 1e-4f;
//    constexpr auto tol = 1e-3f;
//
//    auto v0 = vel();
//    auto a = resistance();
//    if (mass() == 0.f)
//        a = glm::vec3(0.f);
//    else
//    {
//        auto f = force() - glm::vec3(a.x*v0.x*v0.x, a.y*v0.y*v0.y, a.z*v0.z*v0.z);
//        a = f / mass();
//    }
//    auto p = pos();
//    auto v = v0 + a * dt;
//    item::Surface* tar = nullptr;
//    auto tar_n = glm::vec3();
//    auto tc = dt;
//    for (auto &o : others)
//    {
//        if (o->collision() == CollisionShape::Surface)
//        {
//            auto plane = dynamic_cast<item::Surface*>(o.get());
//            auto n = plane->norm();
//
//            // make sure that the norm line always points towards to the moving object
//            auto d = glm::dot(p - plane->pos(), n);
//            if (d < 0.f)
//            {
//                d *= -1.f;
//                n *= -1.f;
//            }
//
//            auto proj = glm::dot(n, v);
//
//            d -= scal().x;
////            std::cout << d << std::endl;
//            if (d < tol)
//            {
//                auto delta = p - plane->pos();
//                if (std::abs(glm::dot(delta, plane->tangentX())) < plane->scal().x+edge_eps &&
//                    std::abs(glm::dot(delta, plane->tangentZ())) < plane->scal().z+edge_eps)
//                {
//                    if (d < -tol)
//                    {
//                        // have collided already
//                        // push the object outside
//                        tc = d/proj;
//                        tar = plane;
//                    }
//                    else if (proj <= 0.f) // collide only if the object is moving towards the plane
//                    {
//                        // just collide or almost collide
//                        v -= n * proj;
//                        v *= 1 - plane->friction();
//                        if (std::abs(v.x) < tol) v.x = 0.f;
//                        if (std::abs(v.y) < tol) v.y = 0.f;
//                        if (std::abs(v.z) < tol) v.z = 0.f;
//                        tc = 0.f;
//                        tar = nullptr;
//                    }
//                }
//            }
//            else if (proj < 0.f) // collide only possible if the object is moving towards the plane
//            {
//                auto movement = -proj * dt;
//                if (movement > d)
//                {
//                    auto ac = glm::dot(a, n);
//                    auto bc = glm::dot(v0, n);
//                    auto cc = scal().x - glm::dot(p - plane->pos(), n);
//                    auto delta = bc*bc+4.f*ac*cc;
//                    if (delta >= 0.f)
//                    {
//                        float t_current = -0.5f* (bc + std::sqrt(delta))/ac;
//                        if (t_current < tc && t_current >= 0.f)
//                        {
//                            auto delta = p + (v0+a*t_current)*t_current - plane->pos();
//                            if (std::abs(glm::dot(delta, plane->tangentX())) < plane->scal().x+edge_eps &&
//                                std::abs(glm::dot(delta, plane->tangentZ())) < plane->scal().z+edge_eps)
//                            {
//                                tc = t_current;
//                                tar = plane;
//                                tar_n = n;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//    if (tar)
//    {
//        v = v0+a*tc;
//        p += v*tc;
//        dt -= tc;
//        vel(tar->restitution() * glm::reflect(v, tar_n));
//        // for the sake of multiple collision at one update
//        if (dt > 0) return update(dt, others);
//    }
//    else vel(v);
//    pos(p + vel()*dt);
}