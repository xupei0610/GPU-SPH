#include "lamp.hpp"

using namespace px;

const char *shader::Lamp::VERTEX_SHADER =
#include "shaders/glsl/lamp.vs"
;
const char *shader::Lamp::FRAGMENT_SHADER =
#include "shaders/glsl/lamp.fs"
;

shader::Lamp::Lamp()
    : Shader(), vao(0), vbo{0}, n_vertices_(-1), n_instances_(-1), n_indices_(-1)
{}

shader::Lamp::~Lamp()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(4, vbo);
}

void shader::Lamp::init(char *geometry_shader)
{
    glDeleteVertexArrays(1, &vao);
    vao = 0;
    glDeleteBuffers(4, vbo);
    vbo[0] = 0; vbo[1] = 0; vbo[2] = 0; vbo[3] = 0;
    n_vertices_ = 0; n_instances_ = 0; n_indices_ = 0;

    if (geometry_shader)
        Shader::init(VERTEX_SHADER, FRAGMENT_SHADER, geometry_shader);
    else
        Shader::init(VERTEX_SHADER, FRAGMENT_SHADER);

    glGenVertexArrays(1, &vao);
    glGenBuffers(4, vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glEnableVertexAttribArray(0); //vertex
    glVertexAttribDivisor(0, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glEnableVertexAttribArray(1); // position
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glEnableVertexAttribArray(2); // color;
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void shader::Lamp::render(unsigned int gl_draw_mode)
{
    glBindVertexArray(vao);
    glDrawElementsInstanced(gl_draw_mode, n_indices_, index_data_type_, nullptr, n_instances_);
    glBindVertexArray(0);
}

template <typename T>
void shader::Lamp::setVertices(const float *data, unsigned int n_vertices,
                               const T *indices, unsigned int n_indices)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    if (n_vertices == n_vertices_)
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*n_vertices_, data);
    else
    {
        n_vertices_ = n_vertices;
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*n_vertices_, data,
                     GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    auto type = std::is_same<T, unsigned short>::value ? GL_UNSIGNED_SHORT :
                (std::is_same<T, unsigned int>::value ? GL_UNSIGNED_INT : GL_UNSIGNED_BYTE);
    if (n_indices == n_indices_ && type == index_data_type_)
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(T)*n_indices_, indices);
    else
    {
        index_data_type_ = type;
        n_indices_ = n_indices;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(T)*n_indices_, indices,
                     GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
template void shader::Lamp::setVertices(const float *data, unsigned int n_vertices,
                                        const unsigned short *indices, unsigned int n_indices);
template void shader::Lamp::setVertices(const float *data, unsigned int n_vertices,
                                        const unsigned int *indices, unsigned int n_indices);
template void shader::Lamp::setVertices(const float *data, unsigned int n_vertices,
                                        const unsigned char *indices, unsigned int n_indices);

void shader::Lamp::setInstances(const float *pos_data, const float *color_data,
                                unsigned int n_instances)
{
    if (n_instances == n_instances_)
    {
        if (pos_data)
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*n_instances_, pos_data);
        }
        if (color_data)
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                            sizeof(float) * 3 * n_instances_, color_data);
        }
    }
    else
    {
        n_instances_ = n_instances;
        if (pos_data)
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * n_instances_,
                         pos_data,
                         GL_STREAM_DRAW);
        }
        if (color_data)
        {
            glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * n_instances_,
                         color_data,
                         GL_STREAM_DRAW);
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
