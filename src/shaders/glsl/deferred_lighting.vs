R"=====(
#version 330 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 tex_coords_in;

out vec2 tex_coords;

void main()
{
    gl_Position = vec4(vertex, 0.f, 1.f);
    tex_coords = tex_coords_in;
}
)====="
