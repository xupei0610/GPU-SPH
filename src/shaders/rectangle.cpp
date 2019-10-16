#include "shaders/rectangle.hpp"
#include "app.hpp"
#include <glm/gtc/matrix_transform.hpp>

using namespace px;

const char *shader::Rectangle::VERTEX_SHADER =
#include "shaders/glsl/orth_rectangle_with_gaussian.vs"
;

const char *shader::Rectangle::FRAGMENT_SHADER =
#include "shaders/glsl/orth_rectangle_with_gaussian.fs"
;

shader::Rectangle::Rectangle()
    : Shader(), vao(0), vbo(0),
      vertices_{
      // x     y       u     v
        0.0f, 0.0f,   0.0f, 1.0f,
        0.0f, 0.0f,   0.0f, 0.0f,
        0.0f, 0.0f,   1.0f, 0.0f,

        0.0f, 0.0f,   0.0f, 1.0f,
        0.0f, 0.0f,   1.0f, 0.0f,
        0.0f, 0.0f,   1.0f, 1.0f
     }
{}

shader::Rectangle::~Rectangle()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void shader::Rectangle::init()
{
    glDeleteVertexArrays(1, &vao); vao = 0;
    glDeleteBuffers(1, &vbo); vbo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    Shader::init(VERTEX_SHADER, FRAGMENT_SHADER);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void *)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void shader::Rectangle::render(float x, float y, float width, float height,
                               glm::vec4 const &color,
                               unsigned int texture_id)
{
    activate(true);
    glBindVertexArray(vao);

    if (width > 2.0f || width < -2.0f)
    {
        set("proj", glm::ortho(0.0f, static_cast<float>(App::instance()->framebufferWidth()),
                               0.0f, static_cast<float>(App::instance()->framebufferHeight())));
    }
    else
        set("proj", glm::mat4(1.f));

    set("rect_color", color);
    if (texture_id == 0)
    {
        set("use_tex", 0);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture_id);
        set("use_tex", 1);
    }

    vertices_[0] = x;         vertices_[1] = y + height;
    vertices_[4] = x;         vertices_[5] = y;
    vertices_[8] = x + width; vertices_[9] = y;

    vertices_[12] = x;         vertices_[13] = y + height;
    vertices_[16] = x + width; vertices_[17] = y;
    vertices_[20] = x + width; vertices_[21] = y + height;
    

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glDisable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), vertices_, GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    activate(false);
}