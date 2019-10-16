R"=====(
#version 330 core
in VS_OUT
{
    vec3 color;
} primitive;

out vec4 color;

void main()
{
    color = vec4(primitive.color, 1.f);
}
)====="
