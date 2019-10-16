"#version 330 core\n"
"layout (location = 0) in vec2 v;"
"layout (location = 1) in vec2 tex_coords_in;"
""
"out vec2 tex_coords;"
"flat out int use_texture;"
""
"uniform int use_tex;"
"uniform mat4 proj;"
""
"void main()"
"{"
"   gl_Position = proj * vec4(v, 0.0, 1.0);"
"   tex_coords = tex_coords_in;"
"   use_texture = use_tex;"
"}"