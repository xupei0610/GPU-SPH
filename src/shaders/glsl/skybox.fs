R"=====(
#version 330
in vec3 tex_coords;
out vec4 color;

uniform samplerCube box;

void main()
{
    color = texture(box, tex_coords);
}
)====="