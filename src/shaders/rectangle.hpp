#ifndef PX_CG_SHADERS_RECTANGLE_HPP
#define PX_CG_SHADERS_RECTANGLE_HPP

#include "shader.hpp"

namespace px { namespace shader
{
class Rectangle;
}}

class px::shader::Rectangle : public Shader
{
public:
    static const char *VERTEX_SHADER;
    static const char *FRAGMENT_SHADER;

public:
    Rectangle();
    ~Rectangle();

    virtual void init();
    void render(float x, float y, float width, float height,
                glm::vec4 const &color,
                unsigned int texture_id);


protected:
    using Shader::init;

protected:
    unsigned int vao, vbo;
private:
    float vertices_[24];
};

#endif // shader::Rectangle::VERTEX_SHADER