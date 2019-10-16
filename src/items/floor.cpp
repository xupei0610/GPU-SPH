#include "floor.hpp"
#include "shader.hpp"
#include "config.h"
#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace px;

item::Floor::Floor(float x_tiles, float y_tiles, std::string const &texture)
    : Surface(),
      x_tiles(x_tiles), y_tiles(y_tiles), texture_name(texture),
      vao(0), vbo(0), texture{0}
{}

item::Floor::~Floor()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(4, texture);
}

void item::Floor::init()
{
    glDeleteVertexArrays(1, &vao); vao = 0;
    glDeleteBuffers(1, &vbo); vbo = 0;
    glDeleteTextures(4, texture);
    texture[0] = 0; texture[1] = 0; texture[2] = 0; texture[3] = 0;

    float vertices[] = {
            //   vertex        texture         norm          tangent
            // x    y   z      u    v          x   y   z     x   y   z
            -1.f, 0.f,  1.f,     0.f, y_tiles, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f,
            -1.f, 0.f, -1.f,     0.f, 0.f,     0.f, 1.f, 0.f, 1.f, 0.f, 0.f,
             1.f, 0.f, -1.f, x_tiles, 0.f,     0.f, 1.f, 0.f, 1.f, 0.f, 0.f,

            -1.f, 0.f,  1.f,     0.f, y_tiles, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f,
             1.f, 0.f, -1.f, x_tiles, 0.f,     0.f, 1.f, 0.f, 1.f, 0.f, 0.f,
             1.f, 0.f,  1.f, x_tiles, y_tiles, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenTextures(4, texture);

    int w, h, ch;
    auto ptr = stbi_load(((ASSET_PATH "/texture/")+texture_name+"_d.png").c_str(), &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[0], w, h, ptr, RGB, REPEAT);         // diffuse texture
    stbi_image_free(ptr);
    ptr = stbi_load(((ASSET_PATH "/texture/")+texture_name+"_n.png").c_str(), &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[1], w, h, ptr, RGB, REPEAT);         // normal texture
    stbi_image_free(ptr);
    ptr = stbi_load(((ASSET_PATH "/texture/")+texture_name+"_s.png").c_str(), &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[2], w, h, ptr, RGB, REPEAT);         // specular texture
    stbi_image_free(ptr);
    ptr = stbi_load(((ASSET_PATH "/texture/")+texture_name+"_h.png").c_str(), &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[3], w, h, ptr, RGB, REPEAT);         // height/displacement texture
    stbi_image_free(ptr);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);   // vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*11, nullptr);
    glEnableVertexAttribArray(1);   // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*11, (void *)(sizeof(float)*3));
    glEnableVertexAttribArray(2);   // norm
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float)*11, (void *)(sizeof(float)*5));
    glEnableVertexAttribArray(3);   // tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float)*11, (void *)(sizeof(float)*8));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void item::Floor::render(Shader &shader)
{
    shader.set("use_tangent", 1);
    shader.set("material.ambient", glm::vec3(1.f));
    shader.set("material.shininess", 32.f);
    shader.set("material.parallax_scale", 0.f);
    shader.set("material.displace_scale", 0.f);
    shader.set("material.displace_mid", 0.5f);

    shader.set("model", model());

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture[3]);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

