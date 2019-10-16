#ifndef PX_CG_UTILS_RANDOM_HPP
#define PX_CG_UTILS_RANDOM_HPP

#include <random>

namespace px
{
inline float rnd()
{
    static std::uniform_real_distribution<float> rd(0, 1);
    static std::mt19937 sd(std::random_device{}());

    return rd(sd);
}
inline float rnd_np()
{
    std::uniform_real_distribution<float> static rd(-1, 1);
    std::mt19937 static sd(std::random_device{}());

    return rd(sd);
}

}
#endif // PX_CG_UTIL_RANDOM_HPP