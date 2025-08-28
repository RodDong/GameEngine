#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_roughness1;
};

uniform Material material;
uniform bool hasNormalTexture;
uniform bool hasDiffuseTexture;
uniform vec3 color;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    
    // Get normal from normal map if available, otherwise use vertex normal
    vec3 normal = vec3(1, 0, 0);
    if (hasNormalTexture) {
        vec3 normalRGB = normalize(texture(material.texture_normal1, TexCoords).rgb * 2.0 - 1.0);
        normal = normalize(TBN * normalRGB);
    } else {
        normal = normalize(Normal);
    }
    gNormal = normal;
    
    // Get diffuse color
    vec4 diffuseColor = texture(material.texture_diffuse1, TexCoords);
    if(diffuseColor.a < 0.1) // If no texture or transparent
        diffuseColor = vec4(0.95, 0.95, 0.95, 1.0); // Default white color
    if(hasDiffuseTexture){
        gAlbedoSpec.rgb = diffuseColor.rgb;
    }else{
        gAlbedoSpec.rgb = color;
    }
    

    
    // Get specular and roughness
    vec4 specularColor = texture(material.texture_specular1, TexCoords);
    float roughness = texture(material.texture_roughness1, TexCoords).r;
    gAlbedoSpec.a = specularColor.r * (1.0 - roughness); // Combine specular and roughness
}  