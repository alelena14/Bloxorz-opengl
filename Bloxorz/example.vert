#version 330 core

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec3 in_Normal;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * in_Position);
    Normal  = normalize(mat3(transpose(inverse(model))) * in_Normal);
    Color   = in_Color.rgb;

    gl_Position = projection * view * model * in_Position;
}
