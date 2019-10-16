#ifndef PX_CG_SHADERS_SKYBOX_HPP
#define PX_CG_SHADERS_SKYBOX_HPP

#include "shader.hpp"

namespace px { namespace shader
{
class Skybox;
}}

class px::shader::Skybox : public Shader
{
public:
    static const char *VERTEX_SHADER;
    static const char *FRAGMENT_SHADER;
public:
    Skybox();
    ~Skybox() override;

    void init(unsigned char *xp, int xp_w, int xp_h,
              unsigned char *xn, int xn_w, int xn_h,
              unsigned char *yp, int yp_w, int yp_h,
              unsigned char *yn, int yn_w, int yn_h,
              unsigned char *zp, int zp_w, int zp_h,
              unsigned char *zn, int zn_w, int zn_h);
    void render();

protected:
    using Shader::init;

protected:
    unsigned int vao, vbo[2], tex;
};

#endif // PX_CG_SHADERS_SKYBOX_HPP
