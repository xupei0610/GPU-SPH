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

// vertex coordinates, 3D
layout (location = 0) in vec3 vertex_in;
// texture coordinates for current vertex
layout (location = 1) in vec2 tex_coords_in;
// normal line direction
layout (location = 2) in vec3 norm_in;
// tangent line direction, for normal mapping case
layout (location = 3) in vec3 tangent_in;
// use tangent or not
// when use_tangent == 1, normal mapping is used, then normal is picked from material normal texture
uniform int use_tangent;
// material of current object
uniform Material material;
// model matrix
uniform mat4 model;

out vec2 tex_coords;
// all 3D space related parameters will be converted into the world coordinate system
// such that deferred lighting can be done easily
out vec3 normal;
out vec3 position; // 3D position of current vertex
out mat3 TBN; // TBN matrix for normal mapping case

// the global configuration of the scene camera
layout (std140) uniform SceneCamera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

void main()
{
    // pass texture coordinates to fragment shader
    tex_coords = tex_coords_in;

    // displacement mapping
    // displacement mapping verifies the actual position of the vertex
    vec3 vertex = vertex_in;
    if (material.displace_scale != 0.f)
    {
        vec3 dv = texture(material.displace, tex_coords).xyz;
        float df = 0.30*dv.x + 0.59*dv.y + 0.11*dv.z;
        // verify current vertex position
        vertex += (df - material.displace_mid) * material.displace_scale * norm_in;
    }

    // convert normal line into world coordinate system
    mat3 norm_mat = transpose(inverse(mat3(model)));
    if (use_tangent == 1)     // normal mapping
    {
        vec3 T = normalize(norm_mat * tangent_in);
        vec3 N = normalize(norm_mat * norm_in);
        T = normalize(T - dot(T,N)*N);
        vec3 B = cross(N, T);
        TBN = mat3(T, B, N);
    }
    else // pick norm from input data
    {
        normal = norm_mat * norm_in;
    }

    // vertex position in world coordinate system
    position = vec3(model * vec4(vertex, 1.f));

    // set 2D position normally such that the framebuffer can contain data as the output screen
    gl_Position = projection * view * model * vec4(vertex, 1.f);
}
)====="
