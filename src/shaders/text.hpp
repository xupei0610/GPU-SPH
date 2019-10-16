#ifndef PX_CG_SHADERS_TEXT_HPP
#define PX_CG_SHADERS_TEXT_HPP

#include <array>

#include "shader.hpp"

namespace px { namespace shader
{
class Text;
}}

class px::shader::Text : public Shader
{
public:
    enum class Anchor : int
    {
        LeftTop     = 11,
        LeftCenter  = 12,
        LeftBottom  = 13,
        CenterTop   = 21,
        Center      = 22,
        CenterBottom= 23,
        RightTop    = 31,
        RightCenter = 32,
        RightBottom = 33
    };
    struct Character
    {
        unsigned int size_x, size_y;
        int bearing_x, bearing_y;
        long int advance;
        Character(unsigned int size_x, unsigned int size_y,
                  int bearing_x, int bearing_y,
                  long int advance);
    };
    static const char *VERTEX_SHADER;
    static const char *FRAGMENT_SHADER;

    Text();
    ~Text() override;

    virtual void init();
    void render(std::string const &text,
                float x, float y,
                float scale, glm::vec4 const &color, Anchor anchor);

    void setFont(const unsigned char *data, std::size_t data_size,
                 unsigned int default_height);

protected:
    using Shader::init;

protected:
    std::array<unsigned, 95> textures;
    std::vector<Character> chars;

    unsigned int vao, vbo;
};

#endif // PX_CG_SHADERS_TEXT_HPP
