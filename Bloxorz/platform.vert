#version 330 core

layout(location = 0) in vec4 in_Position;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec2 in_TexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * in_Position;

    FragPos = worldPos.xyz;
    Normal  = mat3(transpose(inverse(model))) * in_Normal;
    TexCoord = in_TexCoord;

    gl_Position = projection * view * worldPos;
}
