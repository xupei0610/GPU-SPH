R"=====(
#version 330 core

in vec3 gLight;
in vec3 gPos;
in vec3 gNorm;

struct Material
{
   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
   float shininess;
};

uniform Material material;
out vec4 fragColor;

void main(){

   vec3 L = normalize(gLight);
   vec3 V = normalize(-gPos);
   vec3 H = normalize(L + V);
   vec3 N = normalize(gNorm);

   float diffuse = max(0.f, dot(N, L));
   float spec = pow(max(0.f, dot(N, H)), material.shininess);

   fragColor = vec4(   material.ambient
                    + diffuse * material.diffuse
                    + spec * material.specular,
                    1.f);
}
)====="