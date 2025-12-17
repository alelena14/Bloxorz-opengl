#version 330 core

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec3 in_Normal;

out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 shadowMatrix;
uniform int codCol;

void main()
{
    vec4 worldPos = model * in_Position;

    // pozitie fragment
    fragPos = worldPos.xyz;

    // culoare
    fragColor = in_Color;

    // normale
    fragNormal = mat3(transpose(inverse(model))) * in_Normal;

    if (codCol == 1)
        gl_Position = projection * view * shadowMatrix * worldPos;
    else
        gl_Position = projection * view * worldPos;
}
