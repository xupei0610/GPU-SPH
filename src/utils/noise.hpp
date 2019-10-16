#ifndef PX_CG_UTILS_NOISE_HPP
#define PX_CG_UTILS_NOISE_HPP

#include <vector>
#include "glm.hpp"

namespace px {
    class Noise3;
    class FlowNoise3;

template<class S,class T>
inline S lerp(S const &v0, S const &v1, T t)
{
    return (1-t)*v0 + t*v1;
}

template<>
glm::vec3 lerp(glm::vec3 const &v0, glm::vec3 const &v1, float t)
{
    return t * v1 + (1-t) * dot(v1, v0) * v0;
}

template<class S,class T>
inline S bilerp(S const &v00, S const &v10,
                S const &v01, S const &v11,
                T tx, T ty)
{
    return lerp( lerp(v00, v10, tx), lerp(v01, v11, tx), ty);
}

template<class S,class T>
inline S trilerp(const S& v000, const S& v100,
                 const S& v010, const S& v110,
                 const S& v001, const S& v101,
                 const S& v011, const S& v111,
                 T tx, T ty, T tz)
{
    return lerp(bilerp(v000, v100, v010, v110, tx, ty),
                bilerp(v001, v101, v011, v111, tx, ty),
                tz);
}

template<class S,class T>
inline S quadlerp(const S& v0000, const S& v1000,
                  const S& v0100, const S& v1100,
                  const S& v0010, const S& v1010,
                  const S& v0110, const S& v1110,
                  const S& v0001, const S& v1001,
                  const S& v0101, const S& v1101,
                  const S& v0011, const S& v1011,
                  const S& v0111, const S& v1111,
                  T tx, T ty, T tz, T tw)
{
    return lerp(trilerp(v0000, v1000, v0100, v1100, v0010, v1010, v0110, v1110,
                        tx, ty, tz),
                trilerp(v0001, v1001, v0101, v1101, v0011, v1011, v0111, v1111,
                        tx, ty, tz),
                tw);
}

template<class T> inline T curl_ramp(T r)
{
    if (r <= -1) return -1;
    else if (r >= 1) return 1;
    r = (r+1)*.5;
    return r*r*r*(10+r*(-15+r*6))*2-1;
}

}


class px::Noise3
{
public:
    Noise3(std::size_t samples=256);
    virtual void init();
    virtual void init(std::size_t samples);

    float noise(float x, float y, float z) const;
    inline float noise(glm::vec3 const &p) const {return noise(p.x, p.y, p.z); }

    static glm::vec3 sphericalSample();

protected:
    static const unsigned int n=128;
    std::vector<glm::vec3> basis;
    std::vector<int> perm;

    unsigned int hash_index(int i, int j, int k) const
    { return perm[(perm[(perm[i%n]+j)%n]+k)%n]; }
};

class px::FlowNoise3: public Noise3
{
public:
    FlowNoise3(std::size_t samples=256);
    void init(std::size_t samples) override;
    void update(float t); // period of repetition is approximately 1

    inline glm::vec3 operator()(glm::vec3 s)
    {
        return glm::vec3(noise0(s), noise1(s), noise2(s));
    }
protected:
    std::vector<glm::vec3> original_basis;
    std::vector<float> spin_rate;
    std::vector<glm::vec3> spin_axis;

private:
    inline float noise0(glm::vec3 s) { return noise(s.x, s.y, s.z); }
    inline float noise1(glm::vec3 s) { return noise(s.y + 31.416f, s.z - 47.853f, s.x + 12.793f); }
    inline float noise2(glm::vec3 s) { return noise(s.z - 233.145f, s.x - 113.408f, s.y - 185.31f); }
};


#endif