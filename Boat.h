#ifndef BOAT_CLASS_H
#define BOAT_CLASS_H

#ifndef M_PI  
#define M_PI 3.14159265358979323846  
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Sea.h"
#include "Shader.h"

class Boat {
private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    glm::vec3 position;
    glm::vec3 rotation;
    float width, length, height;

public:
    Boat(float w, float l, float h);
    ~Boat();

    void update(Sea& sea, float deltaTime);
    void draw(Shader& shader);

    void setPosition(float x, float z);
    void moveForward(float speed);
    void rotate(float angle);
};

#endif