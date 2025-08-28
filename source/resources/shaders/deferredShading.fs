#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

uniform int NumPointLights;
#define MAX_POINT_LIGHTS 100
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirLight dirLight;
uniform vec3 cameraPos;
uniform bool shadows;

// Directional light shadow map
float ShadowCalculationDirLight(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.01);
    float shadow = 0.0;

    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 offsetCoord = projCoords.xy + vec2(x, y) * texelSize;
            if (offsetCoord.x >= 0.0 && offsetCoord.x <= 1.0 &&
                offsetCoord.y >= 0.0 && offsetCoord.y <= 1.0)
            {
                float pcfDepth = texture(shadowMap, offsetCoord).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
    }
    shadow /= 9.0;
    return shadow;
}

vec3 ApplyPBRLighting(vec3 normalWorld, vec3 lightDir, vec3 viewDir, float shininess,
                      vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular,
                      vec3 diffuseColor, vec3 specularColor)
{
    float diff = max(dot(normalWorld, lightDir), 0.0);
    vec3 halfVec = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normalWorld, halfVec), 0.0), shininess);
    vec3 ambient = lightAmbient * diffuseColor;
    vec3 diffuse = lightDiffuse * diff * diffuseColor;
    vec3 specular = lightSpecular * spec * specularColor;
    return ambient + diffuse + specular;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float roughness, vec4 fragPosLightSpace)
{
    vec3 lightDir = normalize(light.direction);
    float shininess = 1.0 / pow(0.001 + roughness, 2.0);

    float shadow = shadows ? ShadowCalculationDirLight(fragPosLightSpace, normal, lightDir) : 0.0;
    vec3 color = ApplyPBRLighting(normal, lightDir, viewDir, shininess,
                                  light.ambient, light.diffuse, light.specular,
                                  diffuseColor, specularColor);
    return mix(color, color * 0.3, shadow); // apply shadow
}

vec3 CalcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float roughness)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float shininess = 1.0 / pow(0.001 + roughness, 2.0);
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfVec = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfVec), 0.0), shininess);
    
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // combine results
    vec3 ambient = light.ambient * diffuseColor;
    vec3 diffuse = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec * specularColor;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

void main()
{
    // Retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    float Roughness = 1.0 - Specular; // Convert specular to roughness

    // Calculate fragment position in light space
    vec4 FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    // Calculate lighting
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 result = CalcDirLight(dirLight, Normal, viewDir, Diffuse, vec3(Specular), Roughness, FragPosLightSpace);
    
    for (int i = 0; i < NumPointLights; ++i)
        result += CalcPointLight(pointLights[i], FragPos, Normal, viewDir, Diffuse, vec3(Specular), Roughness);

    // Calculate bright color with a smoother threshold
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    float threshold = 0.2;
    float softThreshold = 0.1; // Controls the smoothness of the transition
    
    float brightness_weight = clamp((brightness - threshold) / softThreshold, 0.0, 1.0);
    BrightColor = vec4(result * brightness_weight, 1.0);
    
    FragColor = vec4(result, 1.0);
}
