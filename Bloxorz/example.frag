#version 330 core

in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform int codCol;

out vec4 outColor;

void main()
{
    // ===== UMBRA =====
    if (codCol == 1)
    {
        outColor = vec4(0.0, 0.0, 0.0, 0.35);
        return;
    }

    // ===== ILUMINARE NORMALA =====
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.3 * fragColor.rgb;
    vec3 diffuse = diff * fragColor.rgb;

    vec3 result = (ambient + diffuse) * lightColor;
    outColor = vec4(result, fragColor.a);
}
