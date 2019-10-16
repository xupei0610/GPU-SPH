R"=====(
#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 position;
layout (location = 2) in vec3 color_in;

out VS_OUT
{
    vec3 color;
} primitive;

layout (std140) uniform SceneCamera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

void main()
{
    gl_Position = projection * view * vec4(vertex + position, 1.f);
    primitive.color = color_in;
}
)====="
