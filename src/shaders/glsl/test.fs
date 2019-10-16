R"=====(
#version 330 core
in vec2 tex_coords;
out vec4 color;

uniform sampler2D ambient_buffer;
uniform sampler2D diffuse_buffer;
uniform sampler2D specular_buffer;
uniform sampler2D position_buffer;
uniform sampler2D normal_buffer;

void main()
{
    color = vec4(texture(normal_buffer, tex_coords).rgb, 1.f);
}
)====="
