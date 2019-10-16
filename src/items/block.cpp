#include "block.hpp"
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace px;

item::Block::Block(px::item::Material &&mat)
    : Item(), vao(0), vbo{0}
{
    std::swap(material, mat);
}

item::Block::Block(const px::item::Material &mat)
        : Item(), vao(0), vbo{0}
{
    material.ambient = mat.ambient;
    material.diffuse = mat.diffuse;
    material.specular = mat.specular;
    material.shininess = mat.shininess;
}

item::Block::~Block()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(3, vbo);
}

glm::vec3 item::Block::pos()
{
    return pos_;
}

void item::Block::pos(glm::vec3 const &p)
{
    pos_ = p;
    updateModel();
}

glm::vec3 item::Block::scal()
{
    return scal_;
}

void item::Block::scal(glm::vec3 const &s)
{
    scal_ = s;
    updateModel();
}

glm::vec3 item::Block::rot()
{
    return rot_;
}

void item::Block::rot(glm::vec3 const &r)
{
    rot_ = r;
    updateModel();
}

void item::Block::init()
{
    glDeleteVertexArrays(1, &vao);
    vao = 0;
    glDeleteBuffers(3, vbo);
    vbo[0] = 0; vbo[1] = 0; vbo[2] = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(3, vbo);

    std::array<glm::vec3, 24> vertices = {
            // top
            glm::vec3(-1.f, 1.f, -1.f), glm::vec3(1.f, 1.f, -1.f),
            glm::vec3(1.f, 1.f, 1.f), glm::vec3(-1.f, 1.f, 1.f),
            // bottom
            glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f, -1.f, -1.f),
            glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f),
            // forward
            glm::vec3(1.f, -1.f, -1.f), glm::vec3(1.f, -1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, -1.f),
            // back
            glm::vec3(-1.f, -1.f, -1.f), glm::vec3(-1.f, -1.f, 1.f),
            glm::vec3(-1.f, 1.f, 1.f), glm::vec3(-1.f, 1.f, -1.f),
            // left
            glm::vec3(-1.f, -1.f, -1.f), glm::vec3(-1.f, 1.f, -1.f),
            glm::vec3(1.f, 1.f, -1.f), glm::vec3(1.f, -1.f, -1.f),
            // right
            glm::vec3(-1.f, -1.f, 1.f), glm::vec3(-1.f, 1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, -1.f, 1.f),
    };
    std::array<glm::vec3, 24> norm = {
            // top
            glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f),
            glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f),
            // bottom
            glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 0.f),
            glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 0.f),
            // forward
            glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f),
            glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f),
            // back
            glm::vec3(-1.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f),
            glm::vec3(-1.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f),
            // left
            glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, -1.f),
            glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, -1.f),
            // right
            glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f),
    };
    std::array<unsigned short, 36> vertex_indices = {
             0,  1,  2,  0,  2,  3,
             4,  5,  6,  4,  6,  7,
             8,  9, 10,  8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23
    };
    n_indices_ = vertex_indices.size();

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*vertex_indices.size(), vertex_indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);   // vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*norm.size(), norm.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);   // norm
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void item::Block::render(Shader &shader)
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

void item::Block::updateModel()
{
    model_ = glm::translate(IDENTITY_MAT, pos()) *
             glm::scale(glm::eulerAngleYXZ(rot_.y, rot_.x, rot_.z), scal());
}