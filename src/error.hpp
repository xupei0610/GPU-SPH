#ifndef PX_CG_ERROR_HPP
#define PX_CG_ERROR_HPP

#include <string>
#include <exception>

namespace px
{
class GLFWError;
class OpenGLError;
class AppError;
}

class px::GLFWError : public std::exception
{
public:
    GLFWError(const std::string &msg, const int code=0);
    ~GLFWError() = default;

    const char *what() const noexcept override;
    const int code() const noexcept;

protected:
    std::string msg;
    int err_code;
};

class px::OpenGLError : public std::exception
{
public:
    OpenGLError(const std::string &msg, const int code=0);
    ~OpenGLError() = default;

    const char *what() const noexcept override;
    const int code() const noexcept;

protected:
    std::string msg;
    int err_code;
};

class px::AppError : public std::exception
{
public:
    AppError(const std::string &msg, const int code=0);
    ~AppError() = default;

    const char *what() const noexcept override;
    const int code() const noexcept;

protected:
    std::string msg;
    int err_code;
};


#endif // PX_CG_ERROR_HPP