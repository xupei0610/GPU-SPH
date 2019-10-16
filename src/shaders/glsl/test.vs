R"=====(
#version 330 core
layout (location = 0) in vec2 v;
layout (location = 1) in vec2 tex;

out vec2 tex_coords;
void main()
{
    tex_coords = tex;
    gl_Position = vec4(v, 0.f, 1.f);
}
)====="
