#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;
uniform float exposure;
uniform bool bloom;

void main()
{ 
    // Sample both textures
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    
    // Add bloom
    hdrColor += bloomColor * 0.2;
        
    // Simple exposure tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    
    // Basic gamma correction
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(hdrColor, 1.0);
}