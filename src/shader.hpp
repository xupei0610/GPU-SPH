#ifndef PX_CG_SHADER_HPP
#define PX_CG_SHADER_HPP

#include <string>
#include "glm.hpp"
#include "opengl.hpp"
#include "error.hpp"

namespace px
{
class Shader;
}

class px::Shader
{
public:
    Shader();
    virtual ~Shader();
    [[noreturn]]
    virtual void error(std::string const &msg);
    [[noreturn]]
    virtual void error(std::string const &msg, int code);

    virtual void destroy();
    virtual void bind(unsigned int pid);

    virtual void activate(bool enable);
    virtual void init(const char vertex_shader[], const char frag_shader[],
                      const char geo_shader[] = nullptr,
                      const char tc_shader[] = nullptr, const char te_shader[] = nullptr);
    virtual void init(const char comp_shader[]);

    unsigned int const programID() const noexcept { return pid_; }

    void set(GLint location, int val) const;
    void set(std::string const &field, int val) const;
    void set(const char field[], int val) const;

    void set(GLint location, float val) const;
    void set(std::string const &field, float val) const;
    void set(const char field[], float val) const;

    void set(GLint location, glm::vec2 const &val) const;
    void set(std::string const &field, glm::vec2 const &val) const;
    void set(const char field[], glm::vec2 const &val) const;

    void set(GLint location, glm::vec3 const &val) const;
    void set(std::string const &field, glm::vec3 const &val) const;
    void set(const char field[], glm::vec3 const &val) const;

    void set(GLint location, glm::vec4 const &val) const;
    void set(std::string const &field, glm::vec4 const &val) const;
    void set(const char field[], glm::vec4 const &val) const;

    void set(GLint location, glm::mat4 const &val) const;
    void set(std::string const &field, glm::mat4 const &val) const;
    void set(const char field[], glm::mat4 const &val) const;

    void bind(GLuint id, int val) const;
    void bind(std::string const &name, int val) const;
    void bind(const char *name, int val) const;
    
    Shader &operator=(Shader const &) = delete;
    Shader &operator=(Shader &&) = delete;

private:
    unsigned int pid_;

};


#endif // PX_CG_SHADER_HPP
