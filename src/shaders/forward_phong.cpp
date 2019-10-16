#include "forward_phong.hpp"
#include "config.h"
#include <iostream>

using namespace px;

const char *shader::ForwardPhong::VERTEX_SHADER =
#include "shaders/glsl/simple_phong.vs"
;
const char *shader::ForwardPhong::FRAGMENT_SHADER =
#include "shaders/glsl/simple_phong.fs"
;

shader::ForwardPhong::ForwardPhong()
        : Shader()
{}

void shader::ForwardPhong::init()
{
    std::cout << std::flush;
    Shader::init(VERTEX_SHADER, FRAGMENT_SHADER);
}
