#include "noise.hpp"
#include "random.hpp"

#include "glm.hpp"
#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

using namespace px;


glm::vec3 Noise3::sphericalSample()
{
    glm::vec3 v;
    float m2;
    do {
        v.x = rnd_np();
        v.y = rnd_np();
        v.z = rnd_np();
        m2 = glm::length2(v);
    }
    while (m2>1 || m2 == 0);
    return v / std::sqrt(m2);
}

Noise3::Noise3(std::size_t samples)
{
    init(samples);
}

void Noise3::init()
{
    init(perm.size());
}

void Noise3::init(std::size_t samples)
{
    if (samples == 0) return;

    basis.resize(samples);
    perm.resize(samples);

    for (auto i=0, n = static_cast<int>(samples); i < n; ++i)
    {
        basis[i] = sphericalSample();
        perm[i] = i;
    }

    for (std::size_t i=0, n = samples; i < n; ++i)
    {
        auto j = static_cast<int>(rnd() * (i+1));
        std::swap(perm[i], perm[j]);
    }
}

float Noise3::noise(float x, float y, float z) const
{
    auto floorx = std::floor(x); auto i = static_cast<int>(floorx);
    auto floory = std::floor(y); auto j = static_cast<int>(floory);
    auto floorz = std::floor(z); auto k = static_cast<int>(floorz);
    const glm::vec3 &n000 = basis[hash_index(i,j,k)];
    const glm::vec3 &n100 = basis[hash_index(i+1,j,k)];
    const glm::vec3 &n010 = basis[hash_index(i,j+1,k)];
    const glm::vec3 &n110 = basis[hash_index(i+1,j+1,k)];
    const glm::vec3 &n001 = basis[hash_index(i,j,k+1)];
    const glm::vec3 &n101 = basis[hash_index(i+1,j,k+1)];
    const glm::vec3 &n011 = basis[hash_index(i,j+1,k+1)];
    const glm::vec3 &n111 = basis[hash_index(i+1,j+1,k+1)];
    auto fx = x - floorx; auto sx = fx*fx*fx*(10-fx*(15-fx*6));
    auto fy = y - floory; auto sy = fy*fy*fy*(10-fy*(15-fy*6));
    auto fz = z - floorz; auto sz = fz*fz*fz*(10-fz*(15-fz*6));
    return trilerp(    fx*n000[0] +     fy*n000[1] +     fz*n000[2],
                   (fx-1)*n100[0] +     fy*n100[1] +     fz*n100[2],
                       fx*n010[0] + (fy-1)*n010[1] +     fz*n010[2],
                   (fx-1)*n110[0] + (fy-1)*n110[1] +     fz*n110[2],
                       fx*n001[0] +     fy*n001[1] + (fz-1)*n001[2],
                   (fx-1)*n101[0] +     fy*n101[1] + (fz-1)*n101[2],
                       fx*n011[0] + (fy-1)*n011[1] + (fz-1)*n011[2],
                   (fx-1)*n111[0] + (fy-1)*n111[1] + (fz-1)*n111[2],
                   sx, sy, sz);
}

FlowNoise3::FlowNoise3(std::size_t samples)
    : Noise3(0)
{
    init(samples);
}

void FlowNoise3::init(std::size_t samples)
{
    Noise3::init(samples);

    auto n = perm.size();
    original_basis.resize(n);
    spin_axis.resize(n);
    spin_rate.resize(n);
    for (decltype(n) i=0; i < n; ++i)
    {
        original_basis[i] = basis[i];
        spin_axis[i] = sphericalSample();
        spin_rate[i] = 3.1415926535f*2.f*rnd_np();
    }
}

void FlowNoise3::update(float t)
{
    for (std::size_t i = 0, n = perm.size(); i < n; ++i)
    {
        auto theta=spin_rate[i]*t;
        auto c = std::cos(theta); auto s=std::sin(theta);
        
        auto R00=c+(1-c)*spin_axis[i][0]*spin_axis[i][0];
        auto R01=(1-c)*spin_axis[i][0]*spin_axis[i][1]-s*spin_axis[i][2];
        auto R02=(1-c)*spin_axis[i][0]*spin_axis[i][2]+s*spin_axis[i][1];
        auto R10=(1-c)*spin_axis[i][0]*spin_axis[i][1]+s*spin_axis[i][2];
        auto R11=c+(1-c)*spin_axis[i][1]*spin_axis[i][1];
        auto R12=(1-c)*spin_axis[i][1]*spin_axis[i][2]-s*spin_axis[i][0];
        auto R20=(1-c)*spin_axis[i][0]*spin_axis[i][2]-s*spin_axis[i][1];
        auto R21=(1-c)*spin_axis[i][1]*spin_axis[i][2]+s*spin_axis[i][0];
        auto R22=c+(1-c)*spin_axis[i][2]*spin_axis[i][2];
        basis[i][0]=R00*original_basis[i][0] + R01*original_basis[i][1] + R02*original_basis[i][2];
        basis[i][1]=R10*original_basis[i][0] + R11*original_basis[i][1] + R12*original_basis[i][2];
        basis[i][2]=R20*original_basis[i][0] + R21*original_basis[i][1] + R22*original_basis[i][2];
    }
}

