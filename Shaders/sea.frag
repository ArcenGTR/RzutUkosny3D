#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 ViewPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float time;

void main() {
    // Sea colors
    vec3 deepColor = vec3(0.0, 0.1, 0.4);
    vec3 shallowColor = vec3(0.0, 0.5, 0.8);
    
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
    float specularStrength = 0.8;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Dynamic water color based on depth and normal
    float depth = clamp(1.0 - FragPos.y * 0.1, 0.0, 1.0);
    vec3 waterColor = mix(shallowColor, deepColor, depth);
    
    // Add wave pattern
    float waveFactor = sin(TexCoord.x * 20.0 + time) * sin(TexCoord.y * 20.0 + time) * 0.05;
    waterColor += vec3(waveFactor);
    
    // Final color calculation
    vec3 result = (ambient + diffuse) * waterColor + specular;
    
    // Add foam at wave peaks
    float foamFactor = smoothstep(0.3, 0.35, diff);
    result = mix(result, vec3(0.9, 0.95, 1.0), foamFactor * 0.3);
    
    FragColor = vec4(result, 0.9); // Slight transparency
}