#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform vec4 uColor;
uniform sampler2D uTexture;
uniform int useTexture;
uniform int disableLighting;

uniform vec3 lightPos = vec3(0.0, 50.0, 50.0);
uniform vec3 viewPos; // Pozycja kamery (widza)
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 lightingResult;

    if (disableLighting == 1) {
        lightingResult = vec3(1.0);
    } else {
        vec3 ambient = 0.1 * lightColor;

        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = spec * lightColor * 0.5;

        lightingResult = ambient + diffuse + specular;
    }

    if (useTexture == 1) {
        vec4 texColor = texture(uTexture, TexCoords);
        FragColor = vec4(lightingResult * texColor.rgb, texColor.a);
    } else {
        FragColor = vec4(lightingResult * uColor.rgb, uColor.a);
    }
}
