#include "deferred_lighting.hpp"
#include "config.h"
#include <iostream>

using namespace px;

const char *shader::DeferredLightingPass::VERTEX_SHADER =
#include "shaders/glsl/deferred_lighting_pass.vs"
;
const char *shader::DeferredLightingPass::FRAGMENT_SHADER=
#include "shaders/glsl/deferred_lighting_pass.fs"
;
const char *shader::DeferredLighting::VERTEX_SHADER =
#include "shaders/glsl/deferred_lighting.vs"
;
const char *shader::DeferredLighting::FRAGMENT_SHADER =
#include "shaders/glsl/deferred_lighting.fs"
;
#ifndef LIGHTING_BATCH_SIZE
#define LIGHTING_BATCH_SIZE 0
#endif
const int shader::DeferredLighting::MAX_LIGHTS_PER_BATCH = LIGHTING_BATCH_SIZE;

shader::DeferredLightingPass::DeferredLightingPass()
    : Shader(), fbo(0), rbo(0), buffers{0}
{}

shader::DeferredLightingPass::~DeferredLightingPass()
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(5, buffers);
}

void shader::DeferredLightingPass::init()
{
    glDeleteFramebuffers(1, &fbo); fbo = 0;
    glDeleteRenderbuffers(1, &rbo); rbo = 0;
    glDeleteTextures(5, buffers);
    buffers[0] = 0; buffers[1] = 0; buffers[2] = 0;
    buffers[3] = 0; buffers[4] = 0;

    Shader::init(VERTEX_SHADER, FRAGMENT_SHADER);
    Shader::activate(true);
    set("material.diffuse", 0);
    set("material.normal", 1);
    set("material.specular", 2);
    set("material.displace", 3);
    Shader::activate(false);

    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);
    glGenTextures(5, buffers);

    if (buffer_width_ != 0 && buffer_height_ != 0)
        resize(buffer_width_, buffer_height_);
}

void shader::DeferredLightingPass::resize(int width, int height)
{
    buffer_width_ = width;
    buffer_height_ = height;

    if (fbo == 0)
        return;

#define __TEXTURE_CONFIG_HELPER(i, RGB_or_RGBA)                         \
    glBindTexture(GL_TEXTURE_2D, buffers[i]);                           \
    glTexImage2D(GL_TEXTURE_2D, 0, GL_##RGB_or_RGBA##16F,               \
                 width, height, 0, GL_RGB, GL_FLOAT, 0);                \
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  \
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

#define __FRAMEBUFFER_TEXTURE_BIND_HELPER(i)    \
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+(i), buffers[i], 0)

    __TEXTURE_CONFIG_HELPER(0, RGB);  // ambient_buffer
    __TEXTURE_CONFIG_HELPER(1, RGB);  // diffuse_buffer
    __TEXTURE_CONFIG_HELPER(2, RGBA); // specular_buffer
    __TEXTURE_CONFIG_HELPER(3, RGB);  // position_buffer
    __TEXTURE_CONFIG_HELPER(4, RGB);  // normal_buffer
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    __FRAMEBUFFER_TEXTURE_BIND_HELPER(0);
    __FRAMEBUFFER_TEXTURE_BIND_HELPER(1);
    __FRAMEBUFFER_TEXTURE_BIND_HELPER(2);
    __FRAMEBUFFER_TEXTURE_BIND_HELPER(3);
    __FRAMEBUFFER_TEXTURE_BIND_HELPER(4);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    static constexpr GLenum attach[] =
    {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4
    };
    glDrawBuffers(5, attach);
    // Mac would throw an exception sometime unreasonably
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        error("Failed to generate frame buffer for shader::DeferredLightingPass");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#undef __TEXTURE_CONFIG_HELPER
#undef __FRAMEBUFFER_TEXTURE_BIND_HELPER
}

void shader::DeferredLightingPass::preActivate(bool enable)
{
    if (enable)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void shader::DeferredLightingPass::activate(bool enable)
{
    Shader::activate(enable);
}

void shader::DeferredLightingPass::activateBuffers()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, buffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, buffers[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, buffers[2]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, buffers[3]);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, buffers[4]);
}

void shader::DeferredLightingPass::extractDepthBuffer()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, bufferWidth(), bufferHeight(),
                      0, 0, bufferWidth(), bufferHeight(),
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}


shader::DeferredLighting::DeferredLighting()
    : vao(0), vbo(0), fbo(0), output_buffer(0)
{}

shader::DeferredLighting::~DeferredLighting()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &output_buffer);
}
void shader::DeferredLighting::init()
{
    constexpr static float screen_vertices[] =
    {   //  x       y     u    v
        -1.f,  1.f, 0.f, 1.f,
         1.f,  1.f, 1.f, 1.f,
        -1.f, -1.f, 0.f, 0.f,
         1.f, -1.f, 1.f, 0.f
    };

    glDeleteVertexArrays(1, &vao); vao = 0;
    glDeleteBuffers(1, &vbo); vbo = 0;
    glDeleteFramebuffers(1, &fbo); fbo = 0;
    glDeleteTextures(1, &output_buffer); output_buffer = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &output_buffer);

    std::string tmp(FRAGMENT_SHADER);
    tmp.insert(tmp.find_first_of("c")+4, "\n#define MAX_LIGHTS " + std::to_string(std::max(1, MAX_LIGHTS_PER_BATCH)));

    // Mac would throw an exception sometime without this line.
    // I do not know why.
    std::cout << std::flush;
    Shader::init(VERTEX_SHADER, tmp.c_str());

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void *)(sizeof(float)*2));
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertices), screen_vertices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    Shader::activate(true);
    set("ambient_buffer", 0);
    set("diffuse_buffer", 1);
    set("specular_buffer", 2);
    set("position_buffer", 3);
    set("normal_buffer", 4);
    set("show_only", -1);
    glBindFragDataLocation(programID(), 0, "color");
    Shader::activate(false);

    if (buffer_width_ != 0 && buffer_height_ != 0)
        resize(buffer_width_, buffer_height_);
}

void shader::DeferredLighting::activate(bool enable)
{
    if (enable)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    Shader::activate(enable);
}

void shader::DeferredLighting::renderCache(int n_lights, DeferredLightingPass &pass_shader)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindVertexArray(vao);
    pass_shader.activateBuffers();
    set("n_lights", n_lights);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    // double buffers
    std::swap(output_buffer, pass_shader.buffers[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, output_buffer, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, pass_shader.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pass_shader.buffers[0], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void shader::DeferredLighting::render(int n_lights, DeferredLightingPass &pass_shader)
{
    glBindVertexArray(vao);
    pass_shader.activateBuffers();
    set("n_lights", n_lights);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void shader::DeferredLighting::resize(int width, int height)
{
    buffer_width_ = width;
    buffer_height_ = height;

    if (fbo == 0)
        return;

    glBindTexture(GL_TEXTURE_2D, output_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, output_buffer, 0);
    static constexpr GLenum draw_buffer[] = { GL_COLOR_ATTACHMENT6 };
    glDrawBuffers(1, draw_buffer);
    // Mac would throw an exception sometime unreasonably
//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        error("Failed to generate frame buffer for shader::DeferredLighting");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
