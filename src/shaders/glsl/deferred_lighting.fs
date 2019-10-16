R"=====(
#version 330 core

// texture coordinates
// in deferred lighting
// the output is directly the final screen
// we do lighting related computation only for points those will be rendered
in vec2 tex_coords;

// output fragment color for each point
out vec3 color;

// the global configuration of the scene camera
layout (std140) uniform SceneCamera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

// ambient color
uniform sampler2D ambient_buffer;
// diffuse color
uniform sampler2D diffuse_buffer;
// vec4, specular color + shininess
uniform sampler2D specular_buffer;
// 3D position of the current sampling point
uniform sampler2D position_buffer;
// normal direction
uniform sampler2D normal_buffer;

// struct of point light
struct PointLight
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 coef;
};
// lighting source cache
uniform PointLight lights[MAX_LIGHTS];
// actual number of lights in current batch
uniform int n_lights;


// uniform int show_only;

void main()
{
    // pick out attributes for current point being processed from frame buffers
    vec3 ambient = texture(ambient_buffer, tex_coords).rgb;
    vec3 diffuse = texture(diffuse_buffer, tex_coords).rgb;
    vec4 specular_tmp = texture(specular_buffer, tex_coords).rgba;
    vec3 specular = specular_tmp.rgb;
    float shininess = specular_tmp.a;
    vec3 position = texture(position_buffer, tex_coords).rgb;
    vec3 normal = texture(normal_buffer, tex_coords).rgb;

    // if (show_only == 0)
    // {
    //     color = ambient;
    //     return;
    // }
    // else if (show_only == 1)
    // {
    //     color = diffuse;
    //     return;
    // }
    // else if (show_only == 2)
    // {
    //     color = specular;
    //     return;
    // }
    // else if (show_only == 3)
    // {
    //     color = position;
    //     return;
    // }
    // else if (show_only == 4)
    // {
    //     color = normal;
    //     return;
    // }
    // compute lighting
    vec3 c = vec3(0.f, 0.f, 0.f); // accumulated color
    for (int i = 0; i < n_lights; ++i)
    {   
        // light line, from point to light source
        vec3 L = lights[i].position - position;
        float dist = length(L);

        float atten = 1.f;
        if (lights[i].coef.x != 0.f || lights[i].coef.y != 0.f || lights[i].coef.z != 0.f)
        {
            // effective lighting radius, ignore those lights who are too far away
            // float max_light = max(max(lights[i].diffuse.x, lights[i].diffuse.y), lights[i].diffuse.z);
            // float effective_radius = sqrt(lights[i].coef.y*lights[i].coef.y - 4*lights[i].coef.z*(lights[i].coef.x - (256.0f/5.0f)*max_light));
            // effective_radius -= lights[i].coef.z;
            // effective_radius /= 2.f*lights[i].coef.z;
            // if (dist > effective_radius) continue;

            // attenuation coefficient
            atten /= lights[i].coef.x + lights[i].coef.y*dist + lights[i].coef.z*dist*dist;
        }

        // phong shading
        L /= dist; // light line direction
        vec3 R = reflect(-L, normal); // reflection light direction
        float d = max(dot(normal, L), 0.f); // diffuse coefficient
        float s = pow(max(dot(normalize(camera_position - position), R), 0.f), shininess); // specular coefficient

        // accumulate lighting color
        c += (lights[i].ambient*diffuse + lights[i].diffuse*d*diffuse + lights[i].specular*s*specular) * atten;
        
    }

    color = c + ambient;
}
)====="
