#include "Boat.h"

Boat::Boat(float w, float l, float h) {
    
    width = w;
    length = l;
    height = h;
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation = glm::vec3(0.0f, 0.0f, 0.0f);

    // Create a simple boat mesh (box with pointed front)
    float hw = width / 2.0f;
    float hl = length / 2.0f;

    // Boat vertices
    vertices = {
        // Bottom vertices
        -hw, 0.0f, -hl,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,  // 0
            hw, 0.0f, -hl,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,  // 1
            hw, 0.0f,  hl,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,  // 2
        -hw, 0.0f,  hl,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,  // 3

        // Top vertices
        -hw, height, -hl,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,  // 4
            hw, height, -hl,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,  // 5
            hw, height,  hl,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,  // 6
        -hw, height,  hl,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // 7

        // Pointed front
            0.0f, 0.0f, -hl - hl / 2,   0.0f, -1.0f, -1.0f,   0.5f, 0.0f,  // 8
            0.0f, height, -hl - hl / 2,   0.0f, 1.0f, -1.0f,   0.5f, 0.0f   // 9
    };

    // Boat indices
    indices = {
        // Bottom
        0, 1, 3, 1, 2, 3,

        // Sides
        0, 4, 1, 1, 4, 5,  // Back
        1, 5, 2, 2, 5, 6,  // Right
        2, 6, 3, 3, 6, 7,  // Front
        3, 7, 0, 0, 7, 4,  // Left

        // Top
        4, 7, 5, 5, 7, 6,

        // Point front
        0, 8, 1,       // Bottom triangle
        4, 9, 5,       // Top triangle
        0, 4, 8, 4, 9, 8, // Left side
        1, 8, 5, 5, 8, 9  // Right side
    };

    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);  
}

Boat::~Boat() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Boat::update(Sea& sea, float deltaTime) {
    // Update boat position and rotation based on sea waves
    float waveHeight = sea.getWaveHeight(position.x, position.z);
    glm::vec3 waveNormal = sea.getWaveNormal(position.x, position.z);

    // Set boat height to follow the wave
    position.y = waveHeight;

    // Calculate pitch and roll from wave normal
    float pitch = asin(-waveNormal.z) * 180.0f / M_PI;
    float roll = asin(waveNormal.x) * 180.0f / M_PI;

    // Smooth rotation changes
    float rotationSpeed = 5.0f * deltaTime;
    rotation.x = rotation.x * (1.0f - rotationSpeed) + pitch * rotationSpeed;
    rotation.z = rotation.z * (1.0f - rotationSpeed) + roll * rotationSpeed;
}

void Boat::draw(Shader& shader) {
    // Create model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Boat::setPosition(float x, float z) {
    position.x = x;
    position.z = z;
}

void Boat::moveForward(float speed) {
    float radY = glm::radians(rotation.y);
    position.x -= sin(radY) * speed;
    position.z -= cos(radY) * speed;
}

void Boat::rotate(float angle) {
    rotation.y += angle;
}




