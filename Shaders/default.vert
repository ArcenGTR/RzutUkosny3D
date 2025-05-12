#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

out vec3 color;
out vec2 texCoord;
out vec3 normal;
out vec3 currPos;

uniform mat4 model; // Add this
uniform mat4 camMatrix;

void main()
{
    currPos = vec3(model * vec4(aPos, 1.0f));

    // Apply the model, view, and projection transformations
    gl_Position = camMatrix * vec4(currPos, 1.0);

    // Pass the texture coordinates to the fragment shader
    texCoord = aTex;

    // Pass the color to the fragment shader
    color = aColor;

    // Transform the normal to world space
    normal = mat3(transpose(inverse(model))) * aNormal;
}