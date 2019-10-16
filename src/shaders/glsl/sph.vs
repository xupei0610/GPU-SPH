R"=====(
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec3 vert;
layout (location = 3) in float color;

layout (std140) uniform SceneCamera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

const vec3 light_dir = normalize(vec3(1));

out vec3 gLight;
out vec3 gPos;
out vec3 gNorm;

out float gColor;

// uniform mat4 model;

void main(){
   // mat4 MV = view * model;
   // vec4 pos4 = view * (vec4(pos, 0.f) + model * vec4(vert, 1.f));
   vec4 pos4 = view * vec4(pos + vert, 1.f);

   gLight = (view * vec4(light_dir, 0.f)).xyz;
   gPos = pos4.xyz/pos4.w;
   // gNorm = (transpose(inverse(MV)) * vec4(normalize(norm),0.0)).xyz;
   gNorm = (transpose(inverse(view)) * vec4(normalize(norm),0.0)).xyz;
   gl_Position = projection * pos4;

   gColor = color;
}
)====="