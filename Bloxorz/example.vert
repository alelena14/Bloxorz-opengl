#version 330 core

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec3 in_Normal;

out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragPos;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    vec4 worldPos = model * in_Position;

    fragPos = worldPos.xyz;
    fragColor = in_Color;
    fragNormal = mat3(transpose(inverse(model))) * in_Normal;

    // coordonate in spatiul luminii pentru shadow mapping
    fragPosLightSpace = lightSpaceMatrix * worldPos;

    // pozitia normala a fragmentului pe ecran
    gl_Position = projection * view * worldPos;
}
