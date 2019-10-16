#include "text.hpp"
#include "app.hpp"
#include <cstring>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace px;

const char *shader::Text::VERTEX_SHADER =
#include "shaders/glsl/text.vs"
;
const char *shader::Text::FRAGMENT_SHADER =
#include "shaders/glsl/text.fs"
;

shader::Text::Character::Character(unsigned int size_x, unsigned int size_y,
                                   int bearing_x, int bearing_y,
                                   long int advance)
    : size_x(size_x), size_y(size_y),
      bearing_x(bearing_x), bearing_y(bearing_y),
      advance(advance)
{}

shader::Text::Text()
    : vao(0), vbo(0)
{
    std::memset(textures.data(), 0, sizeof(decltype(textures[0]))*textures.size());
}

shader::Text::~Text()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(94, textures.data());
}

void shader::Text::init()
{
    glDeleteVertexArrays(1, &vao); vao = 0;
    glDeleteBuffers(1, &vbo); vbo = 0;

    Shader::init(VERTEX_SHADER, FRAGMENT_SHADER);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*24, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    Shader::activate(true);
    set("text", 0);
    Shader::activate(false);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void shader::Text::setFont(const unsigned char *data, std::size_t data_size,
                           unsigned int default_height)
{
    glDeleteTextures(94, textures.data());
    glGenTextures(94, textures.data());
    chars.clear();
    chars.reserve(94);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Face face;
    FT_Library ft;
    FT_Init_FreeType(&ft);
    FT_New_Memory_Face(ft, reinterpret_cast<const FT_Byte*>(data), data_size, 0, &face);

    FT_Set_Pixel_Sizes(face, 0, default_height);
    for (auto i = 0; i < 95; ++i)
    {
        auto e = FT_Load_Char(face, i+32, FT_LOAD_RENDER);
        if (e != 0) std::cout << "[Warn] " << e << std::endl;

        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     face->glyph->bitmap.width, face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);
        chars.emplace_back(face->glyph->bitmap.width, face->glyph->bitmap.rows,
                           face->glyph->bitmap_left, face->glyph->bitmap_top,
                           face->glyph->advance.x >> 6);
    }
    glBindTexture(GL_TEXTURE_MAG_FILTER, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void shader::Text::render(std::string const &text, float x, float y,
                          float scale, glm::vec4 const &color,
                          shader::Text::Anchor anchor)
{
    auto framebuffer_width = App::instance()->framebufferWidth();
    auto framebuffer_height = App::instance()->framebufferHeight();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    auto width = 0.f, height = 0.f;
    for (auto const &c : text)
    {
        auto i = c - 32;
        if (i < 0 || i > 94) continue;
        width += chars[i].advance * scale;
        if (chars[i].size_y > height)
            height = chars[i].size_y;
    }

    float x_offset;
    switch(static_cast<int>(anchor)/10)
    {
        case 2:
            x_offset = -width*0.5f;
            break;
        case 3:
            x_offset = -width;
            break;
        default:
            x_offset = 0;
            break;
    }
    float y_offset = framebuffer_height - y;
    switch(static_cast<int>(anchor) % 10)
    {
        case 1:
            y_offset -= height*scale;
            break;
        case 2:
            y_offset -= height*scale*0.5f;
            break;
        default:
            break;
    }

    auto projection = glm::ortho(0.f, static_cast<float>(framebuffer_width),
                                 0.f, static_cast<float>(framebuffer_height));

    Shader::activate(true);
    set("text_color", color);
    set("projection", projection);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    for (auto const &c : text)
    {
        auto i = c - 32;
        if (i < 0 || i > 94) continue;
        auto x_pos = x_offset + x + chars[i].bearing_x*scale;
        auto y_pos = y_offset - (chars[i].size_y - chars[i].bearing_y)*scale;
        auto w = chars[i].size_x*scale;
        auto h = chars[i].size_y*scale;
        float vertices[] ={
                x_pos,   y_pos+h, 0.f, 0.f,
                x_pos,   y_pos,   0.f, 1.f,
                x_pos+w, y_pos,   1.f, 1.f,
                x_pos,   y_pos+h, 0.f, 0.f,
                x_pos+w, y_pos,   1.f, 1.f,
                x_pos+w, y_pos+h, 1.f, 0.f
        };
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += chars[i].advance*scale;
    }

    Shader::activate(false);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
