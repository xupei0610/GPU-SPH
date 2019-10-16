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

// output buffers
layout (location = 0) out vec3 ambient_buffer;
layout (location = 1) out vec3 diffuse_buffer;
layout (location = 2) out vec4 specular_buffer;
layout (location = 3) out vec3 position_buffer;
layout (location = 4) out vec3 normal_buffer;

// the global configuration of the scene camera
layout (std140) uniform SceneCamera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

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
    diffuse_buffer = texture(material.diffuse, coords).rgb;
    ambient_buffer = global_ambient * diffuse_buffer * material.ambient;
    specular_buffer = vec4(texture(material.specular, coords).rgb, material.shininess);
    position_buffer = position;
    normal_buffer = N;
}
)====="
