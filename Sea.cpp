#include "Sea.h"
#include <cmath>

Sea::Sea(int size, float width) {
        gridSize = size;
        vertexCount = size * size;

        // Generate grid vertices
        vertices.resize(vertexCount * 8); // pos(3) + normal(3) + texCoord(2)
        float halfWidth = width / 2.0f;
        float cellSize = width / (gridSize - 1);

        for (int z = 0; z < gridSize; z++) {
            for (int x = 0; x < gridSize; x++) {
                int index = (z * gridSize + x) * 8;
                float xPos = x * cellSize - halfWidth;
                float zPos = z * cellSize - halfWidth;

                // Position
                vertices[index] = xPos;
                vertices[index + 1] = 0.0f; // Will be updated in updateWaves
                vertices[index + 2] = zPos;

                // Normal (will be updated in updateWaves)
                vertices[index + 3] = 0.0f;
                vertices[index + 4] = 1.0f;
                vertices[index + 5] = 0.0f;

                // Texture coordinates
                vertices[index + 6] = (float)x / (gridSize - 1);
                vertices[index + 7] = (float)z / (gridSize - 1);
            }
        }

        // Generate indices for triangle strips
        indices.reserve((gridSize - 1) * gridSize * 2);
        for (int z = 0; z < gridSize - 1; z++) {
            for (int x = 0; x < gridSize; x++) {
                indices.push_back(z * gridSize + x);
                indices.push_back((z + 1) * gridSize + x);
            }
        }

        // Create VAO, VBO, EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

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

Sea::~Sea() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Sea::updateWaves(float deltaTime) {
    time += deltaTime * waveSpeed;

    // Update wave heights and normals
    for (int z = 0; z < gridSize; z++) {
        for (int x = 0; x < gridSize; x++) {
            int index = (z * gridSize + x) * 8;
            float xPos = vertices[index];
            float zPos = vertices[index + 2];

            // Gerstner wave algorithm (simplified)
            float waveHeight1 = waveHeight * sin(waveFreq * xPos + time);
            float waveHeight2 = waveHeight * 0.5f * sin(waveFreq * 2.0f * zPos + time * 1.3f);

            // Combined wave height
            vertices[index + 1] = waveHeight1 + waveHeight2;

            // Calculate approximate normals
            float dydx = waveHeight * waveFreq * cos(waveFreq * xPos + time);
            float dydz = waveHeight * 0.5f * waveFreq * 2.0f * cos(waveFreq * 2.0f * zPos + time * 1.3f);

            // Normal = normalize((-dydx, 1, -dydz))
            glm::vec3 normal = glm::normalize(glm::vec3(-dydx, 1.0f, -dydz));
            vertices[index + 3] = normal.x;
            vertices[index + 4] = normal.y;
            vertices[index + 5] = normal.z;
        }
    }

    // Update VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
}

void Sea::draw() const {
    glBindVertexArray(VAO);
    for (int i = 0; i < gridSize - 1; i++) {
        glDrawElements(GL_TRIANGLE_STRIP, gridSize * 2, GL_UNSIGNED_INT, (void*)(i * gridSize * 2 * sizeof(unsigned int)));
    }
    glBindVertexArray(0);
}

float Sea::getWaveHeight(float x, float z) const {
    // Calculate wave height at any position
    return waveHeight * sin(waveFreq * x + time) +
        waveHeight * 0.5f * sin(waveFreq * 2.0f * z + time * 1.3f);
}

glm::vec3 Sea::getWaveNormal(float x, float z) const {
    // Calculate wave normal at any position
    float dydx = waveHeight * waveFreq * cos(waveFreq * x + time);
    float dydz = waveHeight * 0.5f * waveFreq * 2.0f * cos(waveFreq * 2.0f * z + time * 1.3f);
    return glm::normalize(glm::vec3(-dydx, 1.0f, -dydz));
}
