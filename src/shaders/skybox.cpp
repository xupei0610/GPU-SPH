#include "skybox.hpp"

using namespace px;

const char *shader::Skybox::VERTEX_SHADER =
#include "shaders/glsl/skybox.vs"
;
const char *shader::Skybox::FRAGMENT_SHADER =
#include "shaders/glsl/skybox.fs"
;

shader::Skybox::Skybox()
        : Shader(), vao(0), vbo{0}, tex(0)
{}

shader::Skybox::~Skybox()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(2, vbo);
    glDeleteTextures(1, &tex);
}

void shader::Skybox::init(unsigned char *right, int xp_w, int xp_h,
                          unsigned char *left, int xn_w, int xn_h,
                          unsigned char *top, int yp_w, int yp_h,
                          unsigned char *bottom, int yn_w, int yn_h,
                          unsigned char *back, int zp_w, int zp_h,
                          unsigned char *front, int zn_w, int zn_h)
{
    static constexpr float vertices[] = {
            // x    y   z
            -1.f, -1.f,  1.f, // 0
             1.f, -1.f,  1.f, // 1
             1.f,  1.f,  1.f, // 2
            -1.f,  1.f,  1.f, // 3
            -1.f, -1.f, -1.f, // 4
             1.f, -1.f, -1.f, // 5
             1.f,  1.f, -1.f, // 6
            -1.f,  1.f, -1.f  // 7
    };
    static constexpr unsigned short indices[] = {
            7, 4, 5,
            5, 6, 7,

            0, 4, 7,
            7, 3, 0,

            5, 1, 2,
            2, 6, 5,

            0, 3, 2,
            2, 1, 0,

            7, 6, 2,
            2, 3, 7,

            4, 0, 5,
            5, 0, 1
    };

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(2, vbo);
    glDeleteTextures(1, &tex);

    glGenVertexArrays(1, &vao);
    glGenBuffers(2, vbo);
    glGenTextures(1, &tex);

    Shader::init(VERTEX_SHADER, FRAGMENT_SHADER);

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, xp_w, xp_h, 0, GL_RGB, GL_UNSIGNED_BYTE, right);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, xn_w, xn_h, 0, GL_RGB, GL_UNSIGNED_BYTE, left);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, yp_w, yp_h, 0, GL_RGB, GL_UNSIGNED_BYTE, top);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, yn_w, yn_h, 0, GL_RGB, GL_UNSIGNED_BYTE, bottom);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, zp_w, zp_h, 0, GL_RGB, GL_UNSIGNED_BYTE, back);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, zn_w, zn_h, 0, GL_RGB, GL_UNSIGNED_BYTE, front);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    activate(true);
    set("box", 0);
    activate(false);
}

void shader::Skybox::render()
{
    glDepthFunc(GL_LEQUAL);

    activate(true);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
    activate(false);

    glDepthFunc(GL_LESS);
}