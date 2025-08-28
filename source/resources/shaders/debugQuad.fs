#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int debugMode = 0; // 0: position, 1: normal, 2: albedo

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;
    
    // Normalize the values to visible range
    if(debugMode == 0) // Position
    {
        // Scale position values to visible range
        color = color * 0.1; // Scale down to make it more visible
    }
    else if(debugMode == 1) // Normal
    {
        // Normals are in -1 to 1 range, convert to 0-1 for display
        // Add 1.0 to shift from [-1,1] to [0,2], then divide by 2 to get [0,1]
        color = (color + 1.0) * 0.5;
    }
    
    FragColor = vec4(color, 1.0);
}