#ifndef PX_CG_SHADERS_FORWARD_PHONG_HPP
#define PX_CG_SHADERS_FORWARD_PHONG_HPP

#include "shader.hpp"

namespace px { namespace shader
{
class ForwardPhong;
}}

class px::shader::ForwardPhong : public Shader
{
public:
    static const char *VERTEX_SHADER;
    static const char *FRAGMENT_SHADER;

    ForwardPhong();
    ~ForwardPhong() override = default;

    virtual void init();

protected:
    using Shader::init;
};
#endif // PX_CG_SHADERS_FORWARD_PHONG_HPP
