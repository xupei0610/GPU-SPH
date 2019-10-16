#include "error.hpp"

using namespace px;


GLFWError::GLFWError(const std::string &msg, const int code)
    : msg(msg), err_code(code)
{}

const char* GLFWError::what() const noexcept
{
    return msg.data();
}

const int GLFWError::code() const noexcept
{
    return err_code;
}


OpenGLError::OpenGLError(const std::string &msg, const int code)
        : msg(msg), err_code(code)
{}

const char* OpenGLError::what() const noexcept
{
    return msg.data();
}

const int OpenGLError::code() const noexcept
{
    return err_code;
}


AppError::AppError(const std::string &msg, const int code)
        : msg(msg), err_code(code)
{}

const char* AppError::what() const noexcept
{
    return msg.data();
}

const int AppError::code() const noexcept
{
    return err_code;
}