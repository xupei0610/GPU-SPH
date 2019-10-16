R"=====(
#version 330 core
in vec2 tex_coords;
uniform sampler2D text;
uniform vec4 text_color;

out vec4 color;

void main()
{
    color = vec4(text_color.xyz, texture(text, tex_coords).r*text_color.a);
}
)====="
