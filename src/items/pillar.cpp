#include "pillar.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace px;

item::Pillar::Pillar(Material &&mat)
        : Item(), vao{0}, vbo{0}, pos_(0.f), scal_(1.f), rot_(0.f)
{
    std::swap(material, mat);
}

item::Pillar::Pillar(Material const &mat)
    : Item(), vao{0}, vbo{0}
{
    material.ambient = mat.ambient;
    material.diffuse = mat.diffuse;
    material.specular = mat.specular;
    material.shininess = mat.shininess;
}

item::Pillar::~Pillar()
{
    glDeleteVertexArrays(3, vao);
    glDeleteBuffers(6, vbo);
}

glm::vec3 item::Pillar::pos()
{
    return pos_;
}

void item::Pillar::pos(glm::vec3 const &p)
{
    pos_ = p;
    updateModel();
}

glm::vec3 item::Pillar::scal()
{
    return scal_;
}

void item::Pillar::scal(glm::vec3 const &s)
{
    scal_ = s;
    updateModel();
}

glm::vec3 item::Pillar::rot()
{
    return rot_;
}

void item::Pillar::rot(glm::vec3 const &r)
{
    rot_ = r;
    updateModel();
}

void item::Pillar::updateModel()
{
    // glm::eulerAngleYXZ(GLM_ANG(rot_.y), GLM_ANG(rot_.x), GLM_ANG(rot_.z))
    model_ = glm::mat4(1.f);
    model_ = glm::translate(Item::IDENTITY_MAT, pos())*glm::scale(model_, scal());
}

void item::Pillar::init()
{
    constexpr auto grid = 32;

    glGenVertexArrays(3, vao);
    vao[0] = 0; vao[1] = 0; vao[2] = 0;
    glDeleteBuffers(6, vbo);
    vbo[0] = 0; vbo[1] = 0; vbo[2] = 0;
    vbo[3] = 0; vbo[4] = 0; vbo[5] = 0;

    glGenVertexArrays(3, vao);
    glGenBuffers(6, vbo);

    std::vector<float> vertex, norm;
    constexpr auto gap = static_cast<float>(2*M_PI / grid);

    vertex.reserve(6*(grid+1));
    norm.reserve(6*(grid+1));
    float x, y;
    for (auto i = 0; i < grid+1; ++i) {
        x = std::cos(i*gap);
        y = std::sin(i*gap);

        vertex.push_back(x);
        vertex.push_back(-1.f);
        vertex.push_back(y);
        norm.push_back(x);
        norm.push_back(0.f);
        norm.push_back(y);

        vertex.push_back(x);
        vertex.push_back(1.f);
        vertex.push_back(y);
        norm.push_back(x);
        norm.push_back(0.f);
        norm.push_back(y);
    }
    glBindVertexArray(vao[0]);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertex.size()*sizeof(float), vertex.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)(0));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, norm.size()*sizeof(float), norm.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)(0));

    vertex.clear(); vertex.reserve(3*(grid+2));
    norm.clear(); norm.reserve(3);
    vertex.push_back(0.f);
    vertex.push_back(1.f);
    vertex.push_back(0.f);
    norm.push_back(0.f);
    norm.push_back(1.f);
    norm.push_back(0.f);
    for (auto i = 0; i < grid+1; ++i)
    {
        x = std::cos(i*gap);
        y = std::sin(i*gap);

        vertex.push_back(x);
        vertex.push_back(1.f);
        vertex.push_back(y);
    }
    glBindVertexArray(vao[1]);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, vertex.size()*sizeof(float), vertex.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)(0));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, norm.size()*sizeof(float), norm.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)(0));

    norm[1] = -1.f;
    for (auto i = 1; i < 3*(grid+2); i+=3)
        vertex[i] = -1.f;
    glBindVertexArray(vao[2]);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, vertex.size()*sizeof(float), vertex.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)(0));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glBufferData(GL_ARRAY_BUFFER, norm.size()*sizeof(float), norm.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    _n_indices = grid+2;
}

#include <iostream>
void item::Pillar::render(Shader &shader)
{
    shader.set("material.ambient", material.ambient);
    shader.set("material.diffuse", material.diffuse);
    shader.set("material.specular", material.specular);
    shader.set("material.shininess", material.shininess);

    shader.set("model", model());

    glBindVertexArray(vao[2]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, _n_indices);
    glBindVertexArray(vao[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, _n_indices*2 - 2);
    glBindVertexArray(vao[1]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, _n_indices);

    glBindVertexArray(0);
}
