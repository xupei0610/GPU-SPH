
#include <glm/glm/ext.hpp>
#include <glm/glm/gtx/matrix_cross_product.inl>

#include "sphere.hpp"
#include "cube.hpp"

using namespace px;

item::rigid::Sphere::Sphere()
    : Sphere(glm::vec3(1.f))
{}

item::rigid::Sphere::Sphere(float r)
        : Sphere(glm::vec3(r))
{}

item::rigid::Sphere::Sphere(glm::vec3 const &r)
    : RigidBody()
{
    scal(r);
}

item::rigid::Sphere::~Sphere()
{}

void item::rigid::Sphere::scal(glm::vec3 const &s)
{
    if (s.x != s.y || s.y != s.z || s.z != s.x)
        error("No support to ellipsoid yet. Sphere must have the same radius along each axis.");
    scal_ = s;
}

glm::vec3 item::rigid::Sphere::scal()
{
    return scal_;
}

item::CollisionShape item::rigid::Sphere::collision()
{
    return CollisionShape::Sphere;
}

void item::rigid::Sphere::update(float dt)
{
    aabb_.first = pos() - scal_;
    aabb_.second = pos() + scal_;
    setMomentOfInertia(glm::mat3(2.f* mass() /3 * scal_.x*scal_.x));
}

void item::rigid::Sphere::constraint(std::vector<std::shared_ptr<px::item::RigidBody>> const &objs)
{
    constexpr float restitution = 0.8;
    constexpr auto t_tol = 1e-6f;

    auto t = -1.f;

    struct Recorder {
        glm::vec3 r;
        int c;
        glm::vec3 n;
        Recorder(glm::vec3 const &r, int c, glm::vec3 const &n)
                : r(r), c(c), n(n) {}
    };
    std::vector<Recorder> res;

    for (auto &o : objs)
    {
        if (o.get() == this || o->collision() == CollisionShape::None || !intersect(aabb_, o->AABB()))
            continue;

        auto r_sum = glm::vec3(0.f);
        auto c = 0;
        glm::vec3 n;

        if (o->collision() == CollisionShape::Cube)
        {
            auto r = o->orient();
            auto p = o->pos();
            auto s = o->scal();

            auto local_p = glm::inverse(r) * (pos() - p);
            auto rad = scal().x;

            auto closest = glm::clamp(local_p, -s, s);
            auto delta = glm::distance(closest, local_p);

            if (delta < rad)
            {
                auto r_i = glm::rotate(r, closest) + p - pos();

                auto v_rel = vel() + glm::cross(angularVel(), r_i);
                v_rel -= o->vel() + glm::cross(o->angularVel(), r_i - p);

                auto tmp_n = -glm::normalize(r_i);
                auto v_n = glm::dot(v_rel, tmp_n);

                if (v_n < 0)
                {
                    auto cur_t = (delta - rad)/v_n;
                    if (std::abs(cur_t - t) < t_tol)
                    {
                        r_sum = r_i;
                        c = 1;
                        n = tmp_n;
                    }
                    else if (cur_t > t)
                    {
                        r_sum = r_i;
                        c = 1;
                        n = tmp_n;
                        t = cur_t;
                        res.clear();
                    }
                }

            }
        }
        else if (o->collision() == CollisionShape::Sphere)
        {
            glm::vec3 delta = pos() - o->pos();
            auto d = glm::length(delta);
            if (d < scal().x + o->scal().x)
            {
                auto tmp_n = delta / d;
                auto r_i = -tmp_n*scal().x;

                auto v_rel = vel() + glm::cross(angularVel(), r_i);
                v_rel -= o->vel() + glm::cross(o->angularVel(), r_i + delta);

                auto v_n = glm::dot(v_rel, n);
                if (v_n < 0)
                {
                    auto cur_t = (d-scal().x)/v_n;
                    if (std::abs(cur_t - t) < t_tol)
                    {
                        r_sum = r_i;
                        c = 1;
                        n = tmp_n;
                    }
                    else if (cur_t > t)
                    {
                        r_sum = r_i;
                        c = 1;
                        n = tmp_n;
                        t = cur_t;
                        res.clear();
                    }
                }
            }

        }
        else // if (o->collision() == CollisionShape::Plane)
        {
            auto delta = pos() - o->pos();
            n = glm::rotate(o->orient(), Y_AXIS);
            if (glm::dot(delta, n) < 0.f)
                n = -Y_AXIS;
            else
                n = Y_AXIS;

            auto d = glm::dot(delta, n);
            if (d < scal().x)
            {
                auto r_i = -scal().x * n;

                auto v_rel = vel() + glm::cross(angularVel(), r_i);
                v_rel -= o->vel() + glm::cross(o->angularVel(), r_i + delta);

                auto v_n = glm::dot(v_rel, n);
                if (v_n < 0)
                {
                    auto cur_t = (d-scal().x)/v_n;
                    if (std::abs(cur_t - t) < t_tol)
                    {
                        r_sum = r_i;
                        c = 1;
                    }
                    else if (cur_t > t)
                    {
                        r_sum = r_i;
                        c = 1;
                        t = cur_t;
                        res.clear();
                    }
                }
            }
        }
        if (c > 0)
            res.push_back(Recorder(r_sum, c, n));
    }

    auto dv = glm::vec3(0.f);
    auto dq = glm::vec3(0.f);
    for (auto &s : res)
    {
        auto r_i = s.r / static_cast<float>(s.c);
        auto v_i = vel() + glm::cross(angularVel(), r_i);
        auto R_i = glm::matrixCross3(r_i);
        auto K = glm::mat3(1/mass()) - R_i*I_inv_t*R_i;
        auto j = glm::inverse(K)*(-v_i - restitution * glm::dot(v_i, s.n) * s.n);
        dv += j/mass();
        dq += glm::cross(r_i, j);
    }

    vel(vel() + dv);
    angularVel(angularVel()+I_inv*dq);
}

void item::rigid::Sphere::contact(std::vector<std::shared_ptr<px::item::RigidBody>> const &objs)
{
    auto tol = 1e-4f;
    auto v_tol = 5e-4f;

    for (auto &o : objs)
    {
        if (o.get() == this || o->collision() == CollisionShape::None || !intersect(aabb_, o->AABB()))
            continue;

        if (o->collision() == CollisionShape::Cube)
        {
            // TODO
        }
        else if (o->collision() == CollisionShape::Sphere)
        {
            // TODO
        }
        else // if (o->collision() == CollisionShape::Plane)
        {
            auto delta = pos() - o->pos();
            auto n = glm::rotate(o->orient(), Y_AXIS);
            if (glm::dot(delta, n) < 0.f)
                n = -Y_AXIS;
            else
                n = Y_AXIS;

            auto d = glm::dot(delta, n) - o->scal().x;
            if (std::abs(d) < tol)
            {
                auto v_n = glm::dot(vel(), n);
                if (v_n < v_tol)
                    vel(vel() - v_n*n);
            }
        }
    }
}

