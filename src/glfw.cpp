#include "glfw.hpp"
#include "error.hpp"

using namespace px;

void glfw::init()
{
    static auto glfw_error_callback = [](int code, const char *msg)
    {
        throw GLFWError("Error " + std::to_string(code) + ": " + std::string(msg));
    };
    if (!glfwInit())
        throw GLFWError("Failed to initialize GLFW");

    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void glfw::terminate()
{
    glfwTerminate();
}
