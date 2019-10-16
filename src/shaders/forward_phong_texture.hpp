#ifndef PX_CG_SHADERS_FORWARD_PHONG_TEXTURE_HPP
#define PX_CG_SHADERS_FORWARD_PHONG_TEXTURE_HPP

#include "shader.hpp"

namespace px { namespace shader
{
class ForwardPhongTexture;
}}

class px::shader::ForwardPhongTexture : public Shader
{
public:
    static const int MAX_LIGHTS; // this should not be larger than the one defined in the shader

    static const char *VERTEX_SHADER;
    static const char *FRAGMENT_SHADER;

    ForwardPhongTexture();
    ~ForwardPhongTexture() override = default;

    void init();

protected:
    using Shader::init;
};
#endif // PX_CG_SHADERS_FORWARD_PHONG_TEXTURE_HPP
