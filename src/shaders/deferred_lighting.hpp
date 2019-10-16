#ifndef PX_CG_SHADERS_DEFERRED_LIGHTING_HPP
#define PX_CG_SHADERS_DEFERRED_LIGHTING_HPP

#include "shader.hpp"

namespace px { namespace shader
{
class DeferredLightingPass;
class DeferredLighting;
}}


class px::shader::DeferredLightingPass : public Shader
{
public:
    static const char *VERTEX_SHADER;
    static const char *FRAGMENT_SHADER;

public:
    DeferredLightingPass();
    ~DeferredLightingPass() override;

    void init();
    void activate(bool enable) override;
    void preActivate(bool enable);

    void activateBuffers();
    // set framebuffer size, call after init before use
    void resize(int width, int height);
    void extractDepthBuffer();

    inline void init(int width, int height) { init(); resize(width, height); }
    inline int bufferWidth() const noexcept { return buffer_width_; }
    inline int bufferHeight() const noexcept { return buffer_height_; }

    friend DeferredLighting;

protected:
    using Shader::init;
protected:
    unsigned int fbo;
    unsigned int rbo;
    unsigned int buffers[5];
private:
    int buffer_width_;
    int buffer_height_;
};

class px::shader::DeferredLighting : public Shader
{
public:
    static const char *VERTEX_SHADER;
    static const char *FRAGMENT_SHADER;

    static const int MAX_LIGHTS_PER_BATCH;

public:
    DeferredLighting();
    ~DeferredLighting() override;

    void init();
    // render into cache buffers
    void renderCache(int n_lights, DeferredLightingPass &pass_shader);
    // render cache buffer to screen with n_lights more new lights
    void render(int n_lights, DeferredLightingPass &pass_shader);

    // set framebuffer size, call after init before use
    void resize(int width, int height);

    void activate(bool enable) override;

protected:
    using Shader::init;
protected:
    unsigned int vao;
    unsigned int vbo;
    unsigned int fbo;
    unsigned int output_buffer;
private:
    int buffer_width_;
    int buffer_height_;
};

#endif // PX_CG_SHADERS_DEFERRED_LIGHTING_HPP
