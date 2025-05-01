#ifndef SEA_CLASS_H
#define SEA_CLASS_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

class Sea {
private:
    GLuint VAO, VBO, EBO;
    int gridSize;
    int vertexCount;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float waveHeight = 2.5f;
    float waveFreq = 0.2f;
    float waveSpeed = 1.0f;
    float time = 0.0f;

public:
    Sea(int size, float width);
    ~Sea();

    void updateWaves(float deltaTime);
    void draw() const;
    float getWaveHeight(float x, float z) const;
    glm::vec3 getWaveNormal(float x, float z) const;
};

#endif