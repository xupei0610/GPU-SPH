#ifndef PX_CG_UTILS_SHAPE_GENERATOR_HPP
#define PX_CG_UTILS_SHAPE_GENERATOR_HPP

#include <tuple>
#include <vector>

namespace px { namespace generator
{
std::pair<std::vector<float>, std::vector<unsigned short> >
        sphere(unsigned int n_grid, float radius);
//std::tuple<
//        // vertex            vertex indices
//        std::vector<float>, std::vector<unsigned short>,
//        // uv                norm
//        std::vector<float>, std::vector<float>,
//        // tangent
//        std::vector<float>
//>
void sphereWithNormUVTangle(unsigned int n_grid, float radius,
                            std::vector<float> &vertex, std::vector<unsigned short> &vertex_order,
                            std::vector<float> &uv, std::vector<float> &norm, std::vector<float>&tangent);
}}


#endif // PX_CG_UTIL_SHAPE_GENERATOR_HPP
