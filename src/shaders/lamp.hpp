#ifndef PX_CG_SHADERS_LAMP_HPP
#define PX_CG_SHADERS_LAMP_HPP

#include "shader.hpp"

namespace px { namespace shader
{
class Lamp;
}}

class px::shader::Lamp : public Shader
{
public:
    const static char *VERTEX_SHADER;
    const static char *FRAGMENT_SHADER;

public:
    Lamp();
    ~Lamp() override;

    void init(char *geometry_shader = nullptr);
    void render(unsigned int gl_draw_mode);

    template <typename T>
    void setVertices(const float *data, unsigned int n_vertices,
                     const T *indices, unsigned int n_indices);
    void setInstances(const float *pos_data, const float *color_data,
                      unsigned int n_instances);

protected:
    using Shader::init;

protected:
    unsigned int vao;
    unsigned int vbo[4];
private:
    unsigned int n_vertices_;
    unsigned int n_instances_;
    unsigned int n_indices_;
    int index_data_type_;
};

#endif // PX_CG_SHADERS_LAMP_HPP
