#version 330 core

in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragPos;
in vec4 fragPosLightSpace;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform int codCol;
uniform sampler2DShadow shadowMap;

out vec4 outColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) return 0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    float bias = 0.005;

    // === Percentage Closer Filtering ===
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            // textureProj va returna 1.0 daca e luminat si 0.0 daca e in umbra
            vec4 shadowCoord = vec4(projCoords.xy + vec2(x, y) * texelSize, projCoords.z - bias, 1.0);
            shadow += textureProj(shadowMap, shadowCoord);
        }    
    }
    
    return 1.0 - (shadow / 9.0);
}

void main()
{
    // ===== TARGET =====
    if (codCol == 2)
    {
        outColor = vec4(1.0, 0.843, 0.0, 1.0);
        return;
    }

    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.3 * fragColor.rgb;
    vec3 diffuse = diff * fragColor.rgb;

    // ===== SHADOW MAPPING =====
    float shadow = ShadowCalculation(fragPosLightSpace);

    vec3 result = ambient + (1.0 - shadow) * diffuse;

    outColor = vec4(result * lightColor, fragColor.a);
}
