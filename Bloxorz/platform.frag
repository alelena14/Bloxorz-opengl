#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 fragPosLightSpace;

uniform sampler2D blockTexture;
uniform vec3 lightPos;
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
    vec3 texColor = texture(blockTexture, TexCoord).rgb;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.3 * texColor;
    vec3 diffuse = diff * texColor;
    
    float shadow = ShadowCalculation(fragPosLightSpace);
    vec3 result = ambient + (1.0 - shadow) * diffuse;

    outColor = vec4(result, 1.0);
}
