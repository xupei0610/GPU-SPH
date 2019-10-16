#ifndef PX_CG_OPENGL_HPP
#define PX_CG_OPENGL_HPP

#include <vector>
#include <GL/glew.h>

#define __STR_HELPER(X) #X
#define STR(X) __STR_HELPER(X)

#define OPENGL_ERROR_CHECK(target_var, Shader_or_Program, LINK_or_COMPILE,      \
                           error_handle_fn)                                     \
{                                                                               \
    GLint __status;                                                             \
    glGet##Shader_or_Program##iv(target_var, GL_COMPILE_STATUS, &__status);     \
    if (__status == GL_FALSE) {                                                 \
        glGetShaderiv(target_var, GL_INFO_LOG_LENGTH, &__status);               \
        std::vector<GLchar> __err_msg(__status);                                \
        glGet##Shader_or_Program##InfoLog(target_var, __status, &__status,      \
                                          &__err_msg[0]);                       \
        error(std::string("Failed to compile " STR(target_var) ": ")            \
            .append(__err_msg.begin(), __err_msg.end()));                       \
    }                                                                           \
}

#define OPENGL_SHADER_COMPILE_HELPER(target_var,                                \
        VERTEX_or_FRAGMENT_or_others,  program_id, shader_code, error_handle_fn)\
{                                                                               \
    target_var = glCreateShader(GL_##VERTEX_or_FRAGMENT_or_others##_SHADER);    \
    glShaderSource(target_var, 1, &shader_code, 0);                             \
    glCompileShader(target_var);                                                \
    OPENGL_ERROR_CHECK(target_var, Shader, COMPILE, error_handle_fn);           \
    glAttachShader(program_id, target_var);                                     \
}

#define OPENGL_TEXTURE_BIND_HELPER(tex_obj_id, width, height, data,             \
                                   RGB_LUMINANCE_or_RGBA, repeat_mode)          \
    glBindTexture(GL_TEXTURE_2D, tex_obj_id);                                   \
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_##repeat_mode);        \
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_##repeat_mode);        \
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);           \
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);           \
    glTexImage2D(GL_TEXTURE_2D, 0, GL_##RGB_LUMINANCE_or_RGBA, width, height,   \
                 0, GL_##RGB_LUMINANCE_or_RGBA, GL_UNSIGNED_BYTE, data);        \
    glGenerateMipmap(GL_TEXTURE_2D);                                            \
    glBindTexture(GL_TEXTURE_2D, 0);

#endif // PX_CG_OPENGL_HPP
