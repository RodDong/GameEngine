#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir = normalize(vec3(-2.0, 4.0, -1.0)); // default fallback
uniform vec3 viewPos;
uniform vec3 arrowColor = vec3(1.0, 0.2, 0.2); // red

void main()
{
    // Simple Lambertian diffuse
    vec3 norm = normalize(Normal);
    vec3 lightColor = vec3(1.0);
    float diff = max(dot(norm, -lightDir), 0.0);

    // ambient + diffuse
    vec3 ambient = 0.2 * lightColor;
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * arrowColor;
    FragColor = vec4(arrowColor, 1.0);
}