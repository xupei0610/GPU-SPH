R"=====(
#version 330 core
layout (location = 0) in vec4 v;
uniform mat4 projection;

out vec2 tex_coords;

void main()
{
    tex_coords = v.zw;
    gl_Position = projection * vec4(v.xy, 0.f, 1.f);
}
)====="
