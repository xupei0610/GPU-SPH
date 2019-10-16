
#include <glm/glm/ext.hpp>
#include <glm/glm/gtx/matrix_cross_product.inl>
#include "cube.hpp"

using namespace px;

const std::array<glm::vec3, 8> item::rigid::Cube::vertices = {
        glm::vec3(-1.f, 1.f, -1.f), glm::vec3(1.f, 1.f, -1.f),
        glm::vec3(1.f, 1.f, 1.f), glm::vec3(-1.f, 1.f, 1.f),
        glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f, -1.f, -1.f),
        glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f),
};

item::rigid::Cube::Cube()
        : Cube(glm::vec3(1.f))
{}

item::rigid::Cube::Cube(float r)
        : Cube(glm::vec3(r))
{}

item::rigid::Cube::Cube(glm::vec3 const &r)
        : RigidBody()
{
    scal(r);
}

item::rigid::Cube::~Cube()
{}

void item::rigid::Cube::scal(glm::vec3 const &s)
{
    scal_ = s;
}

glm::vec3 item::rigid::Cube::scal()
{
    return scal_;
}

item::CollisionShape item::rigid::Cube::collision()
{
    return CollisionShape::Cube;
}

void item::rigid::Cube::update(float dt)
{
    auto model = glm::mat3(glm::scale(glm::mat4_cast(orient()), scal_));
    aabb_.first.x = std::numeric_limits<float>::max();
    aabb_.first.y = std::numeric_limits<float>::max();
    aabb_.first.z = std::numeric_limits<float>::max();
    aabb_.second.x = std::numeric_limits<float>::min();
    aabb_.second.y = std::numeric_limits<float>::min();
    aabb_.second.z = std::numeric_limits<float>::min();
    vertices_.clear();
    for (auto const &v: vertices)
    {
        auto v2 = model * v;
        vertices_.push_back(v2 + pos());
        aabb_.first.x = std::min(aabb_.first.x, vertices_.back().x);
        aabb_.first.y = std::min(aabb_.first.y, vertices_.back().y);
        aabb_.first.z = std::min(aabb_.first.z, vertices_.back().z);
        aabb_.second.x = std::max(aabb_.second.x, vertices_.back().x);
        aabb_.second.y = std::max(aabb_.second.y, vertices_.back().y);
        aabb_.second.z = std::max(aabb_.second.z, vertices_.back().z);
    }
    float I[9] = {
        scal_.y*scal_.y + scal_.z*scal_.z, 0.f, 0.f,
        0.f, scal_.x*scal_.x + scal_.z*scal_.z, 0.f,
        0.f, 0.f, scal_.x*scal_.x + scal_.y*scal_.y
    };
    setMomentOfInertia(2.f*mass()/3.f * 4.f * glm::make_mat3(I));
}

void item::rigid::Cube::constraint(const std::vector<std::shared_ptr<px::item::RigidBody>> &objs)
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

    auto collision_response = [&](glm::vec3 const& dir,
                                  glm::vec3 const& p,
                                  glm::quat const& r,
                                  glm::vec3 const& v,
                                  glm::vec3 const& w,
                                  glm::vec3 const& s)
    {
        int c = 0;
        auto r_sum = glm::vec3(0.f);

        auto n = glm::rotate(r, dir);
        // FIXME only vertex based collision detection
        // should be based on edge in 3D env
        for (const auto &vert : vertices_)
        {
            auto local_p = glm::inverse(r) * (vert - p);

            if (std::abs(local_p.x) <= s.x && std::abs(local_p.z) <= s.z &&
                (std::abs(local_p.y) <= s.y || (s.y == 0.f && local_p.y < s.y)))
            {
                auto r_i = vert - pos();
                auto v_rel = vel() + glm::cross(angularVel(), r_i);
//                v_rel -= v + glm::cross(w, vert - p);
                auto v_n = glm::dot(v_rel, n);

                if (v_n < 0)
                {
                    float d;
                    if (dir == X_AXIS)
                        d = local_p.x - s.x;
                    else if (dir == -X_AXIS)
                        d = s.x - local_p.x;
                    else if (dir == Y_AXIS)
                        d = local_p.y - s.y;
                    else if (dir == -Y_AXIS)
                        d = s.y - local_p.y;
                    else if (dir == Z_AXIS)
                        d = local_p.z - s.z;
                    else //if (dir == -Z_AXIS)
                        d = s.z - local_p.z;

                    auto cur_t = d/v_n;

                    if (std::abs(cur_t - t) < t_tol)
                    {
                        r_sum += r_i;
                        c++;
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

    };

    for (auto &o : objs)
    {
        if (o.get() == this || o->collision() == CollisionShape::None || !intersect(aabb_, o->AABB()))
            continue;

        if (o->collision() == CollisionShape::Cube)
        {
            auto s = o->scal();
            auto r = o->orient();
            auto p = o->pos();
            auto w = o->angularVel();
            auto v = o->vel();

            auto dir = X_AXIS;
            collision_response(dir, p, r, v, w, s);
            dir = -X_AXIS;
            collision_response(dir, p, r, v, w, s);
            dir = Y_AXIS;
            collision_response(dir, p, r, v, w, s);
            dir = -Y_AXIS;
            collision_response(dir, p, r, v, w, s);
            dir = Z_AXIS;
            collision_response(dir, p, r, v, w, s);
            dir = -Z_AXIS;
            collision_response(dir, p, r, v, w, s);
        }
        else if (o->collision() == CollisionShape::Sphere)
        {
            auto p = o->pos();
            auto rad = o->scal().x; // FIXME support only regular sphere

            // FIXME only vertex based collision detection
            // should be based on edge in 3D env
            for (const auto &vert : vertices_)
            {
                auto c = 0;
                auto r_sum = glm::vec3(0.f);
                glm::vec3 n;

                auto d = glm::distance(vert,p);
                if (d < rad)
                {
                    auto r_i = vert - pos();
                    auto tmp_n = vert - p;

                    auto v_rel = vel() + glm::cross(angularVel(), r_i);
//                    v_rel -= o->vel() + glm::cross(o->angularVel(), tmp_n);

                    auto v_n = glm::dot(v_rel, glm::normalize(tmp_n));
                    if (v_n < 0)
                    {
                        auto cur_t = d/v_n;

                        if (std::abs(cur_t - t) < t_tol)
                        {
                            r_sum += r_i;
                            c++;
                            n = tmp_n;
                        }
                        else if (cur_t > t)
                        {
                            r_sum = r_i;
                            c = 1;
                            t = cur_t;
                            n = tmp_n;
                            res.clear();
                        }
                    }
                }
                if (c > 0)
                    res.push_back(Recorder(r_sum, c, n));
            }
        }
        else // if (o->collision() == CollisionShape::Plane)
        {
            auto n = glm::rotate(o->orient(), Y_AXIS);
            if (glm::dot(pos() - o->pos(), n) < 0.f)
                n = -Y_AXIS;
            else
                n = Y_AXIS;
            collision_response(n, o->pos(), o->orient(), o->vel(), o->angularVel(), o->scal());
        }
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


void item::rigid::Cube::contact(const std::vector<std::shared_ptr<px::item::RigidBody>> &objs)
{
    auto tol = 1e-6f;
    auto v_tol = 5e-4f;

    auto contact_response = [&](glm::vec3 const &dir,
                                glm::vec3 const &p,
                                glm::quat const &r,
                                glm::vec3 const &s)
    {
        auto n = glm::rotate(r, dir);
        for (const auto &vert : vertices_)
        {
            auto local_p = glm::inverse(r) * (vert - p);
            float d;
            if (dir == X_AXIS)
            {
                if (std::abs(local_p.y) > s.y || std::abs(local_p.z) > s.z)
                    continue;
                d = local_p.x - s.x;
            }
            else if (dir == -X_AXIS)
            {
                if (std::abs(local_p.y) > s.y || std::abs(local_p.z) > s.z)
                    continue;
                d = s.x - local_p.x;
            }
            else if (dir == Y_AXIS)
            {
                if (std::abs(local_p.x) > s.x || std::abs(local_p.z) > s.z)
                    continue;
                d = local_p.y - s.y;
            }
            else if (dir == -Y_AXIS)
            {
                if (std::abs(local_p.x) > s.x || std::abs(local_p.z) > s.z)
                    continue;
                d = s.y - local_p.y;
            }
            else if (dir == Z_AXIS)
            {
                if (std::abs(local_p.y) > s.y || std::abs(local_p.x) > s.x)
                    continue;
                d = local_p.z - s.z;
            }
            else //if (dir == -Z_AXIS)
            {
                if (std::abs(local_p.y) > s.y || std::abs(local_p.x) > s.x)
                    continue;
                d = s.z - local_p.z;
            }

            if (std::abs(d) < tol)
            {
                auto v_n = glm::dot(vel(), n);
                if (v_n < v_tol)
                    vel(vel() - v_n*n);
            }
        }
    };

    for (auto &o : objs)
    {
        if (o.get() == this || o->collision() == CollisionShape::None)
            continue;

        if (o->collision() == CollisionShape::Cube)
        {
            auto p = o->pos();
            auto r = o->orient();
            auto s = o->scal();

            auto dir = X_AXIS;
            contact_response(dir, p, r, s);
            dir = -X_AXIS;
            contact_response(dir, p, r, s);
            dir = Y_AXIS;
            contact_response(dir, p, r, s);
            dir = -Y_AXIS;
            contact_response(dir, p, r, s);
            dir = Z_AXIS;
            contact_response(dir, p, r, s);
            dir = -Z_AXIS;
            contact_response(dir, p, r, s);
        }
        else if (o->collision() == CollisionShape::Sphere)
        {
            auto rad = o->scal().x;
            for (const auto &vert : vertices_)
            {
                auto n = vert - o->pos();
                auto d = glm::length(n)-rad;
                if (std::abs(d) < tol)
                {
                    auto v_n = glm::dot(vel(), n);
                    if (v_n < v_tol)
                        vel(vel() - v_n*n);
                }
            }
        }
        else // if (o->collision() == CollisionShape::Plane)
        {
            auto n = glm::rotate(o->orient(), Y_AXIS);
            if (glm::dot(pos() - o->pos(), n) < 0.f)
                n = -Y_AXIS;
            else
                n = Y_AXIS;
            contact_response(n, o->pos(), o->orient(), o->scal());
        }
    }
}

