#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

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


uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;       // for directional light

uniform int NumPointLights;
#define MAX_POINT_LIGHTS 100
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirLight dirLight;

uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;
uniform bool usePointLight;

/////////////////////////////////////////////////////
// Directional Light Shadow Calculation
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

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(diffuseTexture, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseTexture, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(diffuseTexture, fs_in.TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    // Directional Light
    vec3 lightDir = normalize(-dirLight.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * dirLight.diffuse;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * dirLight.specular;
    vec3 ambient = dirLight.ambient;

    float shadow = 0.0;
    if (shadows && !usePointLight)
        shadow = ShadowCalculationDirLight(fs_in.FragPosLightSpace, normal, lightDir);

    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
    // Add all point lights
    for (int i = 0; i < NumPointLights; ++i)
    {
        result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir);
    }
    
    FragColor = vec4(result, 1);
}