#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
in vec3 normal;
in vec3 currPos;

uniform sampler2D tex0;
uniform vec4 lightColor;
uniform vec3 lightPos; // Change to vec3 to match C++ code

void main()
{
    vec3 normNormal = normalize(normal);
    vec3 lightDir = normalize(lightPos - currPos);

    float diffuse = max(dot(normNormal, lightDir), 0.0);
    float ambient = 0.2; // Add ambient factor to prevent completely dark areas

    FragColor = texture(tex0, texCoord) * lightColor * (diffuse + ambient);
}