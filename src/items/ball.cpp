#include "ball.hpp"
#include "config.h"
#include "stb_image.h"
#include "utils/shape_generator.hpp"

using namespace px;

item::Ball::Ball(Material &&mat)
        : Sphere(), vao(0), vbo{0}
{
    std::swap(material, mat);
}

item::Ball::Ball(Material const &mat)
    : Sphere(), vao(0), vbo{0}
{
    material.ambient = mat.ambient;
    material.diffuse = mat.diffuse;
    material.specular = mat.specular;
    material.shininess = mat.shininess;
}

item::Ball::~Ball()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(3, vbo);
}

void item::Ball::init()
{
    glDeleteVertexArrays(1, &vao);
    vao = 0;
    glDeleteBuffers(3, vbo);
    vbo[0] = 0; vbo[1] = 0; vbo[2] = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(3, vbo);

    std::vector<float> vertices, uv, norm, tangent;
    std::vector<unsigned short> vertex_indices;
//    std::tie(vertices, vertex_indices, uv, norm, tangent) = generator::sphereWithNormUVTangle(48, 1.f);
    generator::sphereWithNormUVTangle(48, 1.f, vertices, vertex_indices, uv, norm, tangent);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*vertex_indices.size(), vertex_indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);   // vertex
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*norm.size(), norm.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);   // norm
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    n_indices_ = vertex_indices.size();
}

void item::Ball::render(Shader &shader)
{
    shader.set("material.ambient", material.ambient);
    shader.set("material.diffuse", material.diffuse);
    shader.set("material.specular", material.specular);
    shader.set("material.shininess", material.shininess);

    shader.set("model", model());

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, n_indices_, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}



item::TextureBall::TextureBall()
    : Sphere(), vao(0), vbo{0}, texture{0}
{}

item::TextureBall::~TextureBall()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(5, vbo);
    glDeleteTextures(4, texture);
}


void item::TextureBall::init()
{
    glDeleteVertexArrays(1, &vao);
    vao = 0;
    glDeleteBuffers(5, vbo);
    vbo[0] = 0;
    vbo[1] = 0;
    vbo[2] = 0;
    vbo[3] = 0;
    vbo[4] = 0;
    glDeleteTextures(4, texture);
    texture[0] = 0;
    texture[1] = 0;
    texture[2] = 0;
    texture[3] = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(5, vbo);
    glGenTextures(4, texture);

    int w, h, ch;
    auto ptr = stbi_load(ASSET_PATH "/texture/wood_d.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[0], w, h, ptr, RGB, REPEAT);         // diffuse texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/wood_n.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[1], w, h, ptr, RGB, REPEAT);         // normal texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/wood_s.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[2], w, h, ptr, RGB, REPEAT);         // specular texture
    stbi_image_free(ptr);
    ptr = stbi_load(ASSET_PATH "/texture/wood_h.png", &w, &h, &ch, 3);
    OPENGL_TEXTURE_BIND_HELPER(texture[3], w, h, ptr, RGB, REPEAT);         // height/displacement texture
    stbi_image_free(ptr);

    std::vector<float> vertices, uv, norm, tangent;
    std::vector<unsigned short> vertex_indices;
//    std::tie(vertices, vertex_indices, uv, norm, tangent) = generator::sphereWithNormUVTangle(48, 1.f);
    generator::sphereWithNormUVTangle(48, 1.f, vertices, vertex_indices, uv, norm, tangent);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*vertex_indices.size(), vertex_indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);   // vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*uv.size(), uv.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);   // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*norm.size(), norm.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);   // norm
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*tangent.size(), tangent.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);   // tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    n_indices_ = vertex_indices.size();
}

void item::TextureBall::render(Shader &shader)
{
    shader.set("use_tangent", 1);
    shader.set("material.ambient", glm::vec3(1.0f, 1.f, 1.f));
    shader.set("material.shininess", 50.f);
    shader.set("material.parallax_scale", 0.f);
    shader.set("material.displace_scale", 0.1f);
    shader.set("material.displace_mid", 0.2f);

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

    glDrawElements(GL_TRIANGLES, n_indices_, GL_UNSIGNED_SHORT, nullptr);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}