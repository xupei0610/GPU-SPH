R"=====(
#version 330 core

// struct of the material of current object
struct Material
{
    vec3 ambient; // ambient color coefficient related to diffuse color
    sampler2D diffuse; // diffuse mapping
    sampler2D normal;  // normal mapping
    sampler2D specular; // specular mapping
    float shininess;
    sampler2D displace; // displacement mapping
    float displace_scale; // displacement amplification coefficient, 0 for no displacement mapping
    float parallax_scale; // height scale for parallax mapping, 0 for no parallax mapping
    float displace_mid; // mid-point value for displacement/parallax mapping
};
// material of current object
uniform Material material;
// global ambient
uniform vec3 global_ambient;
// use tangent or not
// when use_tangent == 1, normal mapping is used, then normal is picked from material normal texture
uniform int use_tangent;

in vec2 tex_coords;
in vec3 normal;
in vec3 position;
in mat3 TBN;

// output fragment color
out vec4 color;

// the global configuration of the scene camera
layout (std140) uniform SceneCamera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

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

void main()
{
    // parallax mapping
    // parallax mapping change the virtual position of current fragment, not vertex
    vec2 coords = tex_coords;
    if (material.parallax_scale != 0.f)
    {
        // convert displacement mapping into scalar value
        vec3 dv = texture(material.displace, tex_coords).xyz;
        float df = 0.30*dv.x + 0.59*dv.y + 0.11*dv.z - material.displace_mid;
        // view direction
        vec3 V = normalize(camera_position - position);
        // parallax mapping
        coords -= V.xy / V.z * (df * material.parallax_scale);
        if (coords.x < 0.f || coords.y < 0.f || coords.x > 1.f || coords.y > 1.f)
            discard;
    }

    // normal direction
    vec3 N;
    if (use_tangent == 1)   // normal mapping
    {
        N = texture(material.normal, coords).rgb;
        N = normalize(N*2.f - 1.f);
        N = normalize(TBN * N);
    }
    else
        N = normal;

    // same to deferred_lighting
    // but we do lighting computation immediately
    vec3 diffuse = texture(material.diffuse, coords).rgb;
    vec3 ambient = global_ambient * diffuse * material.ambient;
    vec3 specular = texture(material.specular, coords).rgb;
    vec3 c = vec3(0.f); // accumulated color
    for (int i = 0; i < n_lights; ++i)
    {
        if (lights[i].coef.x == 0.f && lights[i].coef.y == 0.f && lights[i].coef.z == 0.f)
            continue;

        // light line, from point to light source
        vec3 L = lights[i].position - position;
        float dist = length(L);

        // attenuation coefficient
        float atten = 1.f;
        if (lights[i].coef.x != 0.f || lights[i].coef.y != 0.f || lights[i].coef.z != 0.f)
        {
            // effective lighting radius, ignore those lights who are too far away
            float max_light = max(max(lights[i].diffuse.x, lights[i].diffuse.y), lights[i].diffuse.z);
            float effective_radius = sqrt(lights[i].coef.y*lights[i].coef.y - 4*lights[i].coef.z*(lights[i].coef.x - (256.0f/5.0f)*max_light));
            effective_radius -= lights[i].coef.z;
            effective_radius /= 2.f*lights[i].coef.z;
            if (dist > effective_radius) continue;

            // attenuation coefficient
            atten /= lights[i].coef.x + lights[i].coef.y*dist + lights[i].coef.z*dist*dist;
        }

        // phong shading
        L /= dist; // light line direction
        vec3 R = reflect(-L, N); // reflection light direction
        float d = max(dot(N, L), 0.f); // diffuse coefficient
        float s = pow(max(dot(normalize(camera_position - position), R), 0.f), material.shininess); // specular coefficient

        // accumulate lighting color
        c += (lights[i].ambient*diffuse + lights[i].diffuse*d*diffuse + lights[i].specular*s*specular) * atten;
    }
    color = vec4(c + ambient, 1.f);
}
)====="
