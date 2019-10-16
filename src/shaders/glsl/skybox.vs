R"=====(
#version 330 core
layout (location = 0) in vec3 v;

out vec3 tex_coords;

layout (std140) uniform SceneCamera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

void main()
{
    tex_coords = v;
    vec4 pos = projection * mat4(mat3(view)) * vec4(v, 1.0);
    gl_Position = pos.xyww;
}
)====="