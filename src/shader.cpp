#include "shader.hpp"
#include <glm/gtc/type_ptr.hpp>

using namespace px;

Shader::Shader()
    : pid_(0)
{}

Shader::~Shader()
{
    destroy();
}

void Shader::error(std::string const &msg)
{
    throw OpenGLError(msg);
}

void Shader::error(std::string const &msg, int code)
{
    throw OpenGLError(msg, code);
}

void Shader::destroy()
{
    glDeleteProgram(pid_);
}

void Shader::bind(unsigned int pid)
{
    pid_ = pid;
}

void Shader::activate(bool enable)
{
    if (enable)
        glUseProgram(programID());
    else
        glUseProgram(0);
}

void Shader::set(GLint location, int val) const
{
    glUniform1i(location, val);
}
void Shader::set(std::string const &field, int val) const
{
    glUniform1i(glGetUniformLocation(programID(), field.c_str()), val);
}
void Shader::set(const char field[], int val) const
{
    glUniform1i(glGetUniformLocation(programID(), field), val);
}

void Shader::set(GLint location, float val) const
{
    glUniform1f(location, val);
}
void Shader::set(std::string const &field, float val) const
{
    glUniform1f(glGetUniformLocation(programID(), field.c_str()), val);
}
void Shader::set(const char field[], float val) const
{
    glUniform1f(glGetUniformLocation(programID(), field), val);
}

void Shader::set(GLint location, glm::vec2 const &val) const
{
    glUniform2fv(location, 1, glm::value_ptr(val));
}
void Shader::set(std::string const &field, glm::vec2 const &val) const
{
    glUniform2fv(glGetUniformLocation(programID(), field.c_str()), 1, glm::value_ptr(val));
}
void Shader::set(const char field[], glm::vec2 const &val) const
{
    glUniform2fv(glGetUniformLocation(programID(), field), 1, glm::value_ptr(val));
}

void Shader::set(GLint location, glm::vec3 const &val) const
{
    glUniform3fv(location, 1, glm::value_ptr(val));
}
void Shader::set(std::string const &field, glm::vec3 const &val) const
{
    glUniform3fv(glGetUniformLocation(programID(), field.c_str()), 1, glm::value_ptr(val));
}
void Shader::set(const char field[], glm::vec3 const &val) const
{
    glUniform3fv(glGetUniformLocation(programID(), field), 1, glm::value_ptr(val));
}

void Shader::set(GLint location, glm::vec4 const &val) const
{
    glUniform4fv(location, 1, glm::value_ptr(val));
}
void Shader::set(std::string const &field, glm::vec4 const &val) const
{
    glUniform4fv(glGetUniformLocation(programID(), field.c_str()), 1, glm::value_ptr(val));
}
void Shader::set(const char field[], glm::vec4 const &val) const
{
    glUniform4fv(glGetUniformLocation(programID(), field), 1, glm::value_ptr(val));
}

void Shader::set(GLint location, glm::mat4 const &val) const
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
}
void Shader::set(std::string const &field, glm::mat4 const &val) const
{
    glUniformMatrix4fv(glGetUniformLocation(programID(), field.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}
void Shader::set(const char field[], glm::mat4 const &val) const
{
    glUniformMatrix4fv(glGetUniformLocation(programID(), field), 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::init(const char *vertex_shader, const char *frag_shader,
                  const char *geo_shader, const char *tc_shader,
                  const char *te_shader)
{
    if (programID() == 0) pid_ = glCreateProgram();

    unsigned int vs, fs, gs = 0, tcs = 0, tes = 0;

    OPENGL_SHADER_COMPILE_HELPER(vs, VERTEX, programID(), vertex_shader, error);
    OPENGL_SHADER_COMPILE_HELPER(fs, FRAGMENT, programID(), frag_shader, error);
    if (geo_shader)
    OPENGL_SHADER_COMPILE_HELPER(gs, GEOMETRY, programID(), geo_shader, error);
    if (tc_shader)
    OPENGL_SHADER_COMPILE_HELPER(tcs, TESS_CONTROL, programID(), tc_shader, error);
    if (te_shader)
    OPENGL_SHADER_COMPILE_HELPER(tes, TESS_EVALUATION, programID(), te_shader, error);

    glLinkProgram(programID());
    OPENGL_ERROR_CHECK(programID(), Program, LINK, error);

    glDetachShader(programID(), vs);
    glDeleteShader(vs);
    glDetachShader(programID(), fs);
    glDeleteShader(fs);
    if (gs != 0)
    {
        glDetachShader(programID(), gs);
        glDeleteShader(gs);
    }
    if (tcs != 0)
    {
        glDetachShader(programID(), tcs);
        glDeleteShader(tcs);
    }
    if (tes != 0)
    {
        glDetachShader(programID(), tes);
        glDeleteShader(tes);
    }
}

void Shader::init(const char *comp_shader)
{
    if (programID() == 0) pid_ = glCreateProgram();

    unsigned int cs;
    OPENGL_SHADER_COMPILE_HELPER(cs, COMPUTE, programID(), comp_shader, error);
    glLinkProgram(programID());
    OPENGL_ERROR_CHECK(programID(), Program, LINK, error);

    glDetachShader(programID(), cs);
    glDeleteShader(cs);
}

void Shader::bind(GLuint id, int val) const
{
    glUniformBlockBinding(programID(), id, val);
}

void Shader::bind(std::string const &name, int val) const
{
    glUniformBlockBinding(programID(), glGetUniformBlockIndex(programID(), name.c_str()), val);
}

void Shader::bind(const char *name, int val) const
{
    glUniformBlockBinding(programID(), glGetUniformBlockIndex(programID(), name), val);
}
