#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    // Wood texture colors
    vec3 woodColor1 = vec3(0.6, 0.3, 0.1);
    vec3 woodColor2 = vec3(0.5, 0.25, 0.08);
    
    // Create a basic wood grain effect
    float woodGrain = sin(FragPos.x * 20.0) * 0.5 + 0.5;
    woodGrain = pow(woodGrain, 2.0);
    vec3 objectColor = mix(woodColor1, woodColor2, woodGrain);
    
    // Normal vector
    vec3 norm = normalize(Normal);
    
    // Light properties
    vec3 lightColor = vec3(1.0, 1.0, 0.9);
    
    // Ambient lighting
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting
    float specularStrength = 0.3;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Final color calculation
    vec3 result = (ambient + diffuse) * objectColor + specular;
    
    FragColor = vec4(result, 1.0);
}