#include "forward_phong_texture.hpp"
#include "config.h"
#include <iostream>

using namespace px;
#ifndef LIGHTING_BATCH_SIZE
#define LIGHTING_BATCH_SIZE 0
#endif
const int shader::ForwardPhongTexture::MAX_LIGHTS = LIGHTING_BATCH_SIZE;

const char *shader::ForwardPhongTexture::VERTEX_SHADER =
#include "shaders/glsl/deferred_lighting_pass.vs"
;
const char *shader::ForwardPhongTexture::FRAGMENT_SHADER =
#include "shaders/glsl/phong_texture.fs"
;

shader::ForwardPhongTexture::ForwardPhongTexture()
    : Shader()
{}

void shader::ForwardPhongTexture::init()
{
    std::string tmp(FRAGMENT_SHADER);
    tmp.insert(tmp.find_first_of("c")+4, "\n#define MAX_LIGHTS " + std::to_string(std::max(1, MAX_LIGHTS)));

    // Mac would throw an exception sometime without this line.
    // I do not know why.
    std::cout << std::flush;
    Shader::init(VERTEX_SHADER, tmp.c_str());

    Shader::activate(true);
    set("material.diffuse", 0);
    set("material.normal", 1);
    set("material.specular", 2);
    set("material.displace", 3);
    Shader::activate(false);
}
