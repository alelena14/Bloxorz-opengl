#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D blockTexture;
uniform vec3 lightPos;

out vec4 outColor;

void main()
{
    vec3 texColor = texture(blockTexture, TexCoord).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.3 * texColor;
    vec3 diffuse = diff * texColor;

    outColor = vec4(ambient + diffuse, 1.0);
}
