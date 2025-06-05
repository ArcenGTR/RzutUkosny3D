// main.cpp
// Kompilacja: g++ main.cpp -lglfw -ldl -lGL -lX11 -lpthread -lXrandr -lXi -lXxf86vm -lXcursor -o rzut
#define M_PI 3.14159265358979323846f

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm> // Dla std::max, std::min
#include <map>       // Dla std::map do przechowywania tekstur

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Do ładowania tekstur
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Dodatkowe makro dla ofsetu, jeśli nie działa `offsetof`
#ifndef offsetof
#define offsetof(st, m) ((size_t)&(((st*)0)->m))
#endif

// Struktury
struct Vec3 {
    float x, y, z;
    Vec3 operator+(const Vec3& b) const { return { x + b.x, y + b.y, z + b.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
    Vec3 operator-(const Vec3& b) const { return { x - b.x, y - b.y, z - b.z }; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalize() const {
        float l = length();
        return l > 0 ? Vec3{ x / l, y / l, z / l } : Vec3{ 0,0,0 };
    }
    float dot(const Vec3& b) const { return x * b.x + y * b.y + z * b.z; }
};

struct Projectile {
    Vec3 pos, vel;
    std::vector<Vec3> trail;
};

// Nowa struktura dla klocków
struct Block {
    Vec3 pos;
    Vec3 vel; // Nadal tu jest, ale nie będzie używane w update
    Vec3 size; // Wymiary klocka (np. 1.0f, 1.0f, 1.0f dla sześcianu)
    float mass; // Nie będzie używane w update
    float restitution; // Nie będzie używane w update
    GLuint textureID; // ID tekstury dla tego konkretnego klocka
};

// Struktura do przechowywania informacji o kolizji
struct CollisionInfo {
    bool collided = false;
    Vec3 normal = { 0,0,0 };
    float penetrationDepth = 0.0f;
};

// Globalne zmienne dla obiektów gry
Projectile proj;
bool isRunning = false;
std::vector<Block> blocks; // Lista klocków

float gravity = 9.81f;
float velocity = 50.0f, angle = 45.0f, mass = 1.0f, drag = 0.01f;
float restitution = 0.6f;
float launchYaw = 0.0f; // Kąt obrotu wokół osi Y dla pocisku

// NOWA ZMIENNA: Współczynnik tłumienia/hamowania (bliżej 0 = szybciej zwalnia, 1.0 = brak spowolnienia)
float dampingFactor = 0.99f; // Domyślnie 0.99f, możesz dostosować

glm::vec3 cameraPos = glm::vec3(0.0f, 20.0f, 100.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float lastX = 400, lastY = 300;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;

bool freeCameraMode = true; // true = tryb swobodnej kamery, false = tryb statyczny
bool zKeyPressedLastFrame = false; // Pomocnicza zmienna do wykrywania naciśnięcia klawisza 'Z'

// Globalna mapa do przechowywania ID tekstur
std::map<std::string, GLuint> textures;
GLuint textureIDProjectile; // ID tekstury dla pocisku

// Globalne zmienne dla VAO/VBO/EBO
GLuint vaoGround = 0, vaoSphere = 0, vboGround = 0, vboSphere = 0, eboSphere = 0;
GLuint vaoBlock = 0, vboBlock = 0;
GLuint shaderProgram = 0;


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (freeCameraMode) { // Tylko jeśli jesteśmy w trybie swobodnej kamery
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}

void processInput(GLFWwindow* window, float deltaTime) {
    // Obsługa klawisza 'Z' do przełączania trybów kamery
    bool zKeyPressed = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;
    if (zKeyPressed && !zKeyPressedLastFrame) {
        freeCameraMode = !freeCameraMode;
        if (freeCameraMode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // Resetujemy lastX i lastY po przełączeniu, aby uniknąć skoku kamery
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2, height / 2);
            lastX = width / 2;
            lastY = height / 2;
            firstMouse = true; // Zresetuj flagę pierwszej myszy
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    zKeyPressedLastFrame = zKeyPressed;

    if (freeCameraMode) { // Tylko jeśli jesteśmy w trybie swobodnej kamery
        float speed = 50.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += speed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= speed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    }
}

float toRadians(float degrees) { return degrees * M_PI / 180.0f; }

// Funkcja do inicjalizacji klocków
void initBlocks() {
    blocks.clear();
    // Stały rozmiar i parametry dla statycznych klocków
    float blockSize = 10.0f;
    // Masa i restytucja dla klocków statycznych nie mają znaczenia
    float blockMass = 10.0f;
    float blockRestitution = 0.5f;

    // Zwiększone odległości X i Z, aby klocki były jeszcze dalej od środka
    // Przypisanie tekstur do klocków
    blocks.push_back({ {30, blockSize * 0.5f, 25}, {0,0,0}, {blockSize, blockSize, blockSize}, blockMass, blockRestitution, textures["textures/placeholder1.jpg"] });
    blocks.push_back({ {-30, blockSize * 0.5f, -25}, {0,0,0}, {blockSize, blockSize, blockSize}, blockMass, blockRestitution, textures["textures/placeholder2.jpg"] });
    blocks.push_back({ {25, blockSize * 0.5f, -30}, {0,0,0}, {blockSize, blockSize, blockSize}, blockMass, blockRestitution, textures["textures/placeholder1.jpg"] });
    blocks.push_back({ {0, blockSize * 0.5f, 30}, {0,0,0}, {blockSize, blockSize, blockSize}, blockMass, blockRestitution, textures["textures/placeholder2.jpg"] });
    blocks.push_back({ {-25, blockSize * 0.5f, 0}, {0,0,0}, {blockSize, blockSize, blockSize}, blockMass, blockRestitution, textures["textures/placeholder1.jpg"] });
}


void reset() {
    float radAngle = toRadians(angle);
    float radYaw = toRadians(launchYaw);

    proj.pos = { 0, 0.5f, 0 };
    proj.vel = {
        velocity * std::cos(radAngle) * std::sin(radYaw),
        velocity * std::sin(radAngle),
        velocity * std::cos(radAngle) * std::cos(radYaw)
    };
    proj.trail.clear();
    isRunning = false;

    initBlocks(); // Resetuj klocki za każdym razem
}

// Funkcja do sprawdzania kolizji kula-AABB (Bounding Box)
// Zwraca CollisionInfo zawierające flagę kolizji, normalną i głębokość penetracji.
CollisionInfo checkCollisionSphereAABB(const Vec3& sphereCenter, float sphereRadius, const Block& block) {
    CollisionInfo info;
    Vec3 blockMin = block.pos - block.size * 0.5f;
    Vec3 blockMax = block.pos + block.size * 0.5f;

    // Najbliższy punkt na AABB do centrum sfery (clampowanie)
    float closestX = std::max(blockMin.x, std::min(sphereCenter.x, blockMax.x));
    float closestY = std::max(blockMin.y, std::min(sphereCenter.y, blockMax.y));
    float closestZ = std::max(blockMin.z, std::min(sphereCenter.z, blockMax.z));

    Vec3 closestPoint = { closestX, closestY, closestZ };
    Vec3 distanceVec = sphereCenter - closestPoint;
    float distanceSq = distanceVec.dot(distanceVec);
    float radiusSq = sphereRadius * sphereRadius;

    if (distanceSq < radiusSq) {
        info.collided = true;

        // Bardziej dokładna normalna kolizji:
        if (distanceVec.length() > 0.0001f) { // Unikamy dzielenia przez zero
            info.normal = distanceVec.normalize();
        }
        else {
            // Jeśli kula jest dokładnie w centrum, wybieramy np. normalną Y (góra/dół)
            info.normal = { 0.0f, 1.0f, 0.0f };
        }
        // Glębokość penetracji to promień sfery minus długość wektora odległości
        info.penetrationDepth = sphereRadius - distanceVec.length();

    }
    return info;
}


void update(float dt) {
    if (!isRunning) return;

    // Fizyka pocisku
    float speed = proj.vel.length();
    float fdrag = drag * speed;
    Vec3 acc = {
        -fdrag * proj.vel.x / mass,
        -gravity - fdrag * proj.vel.y / mass,
        -fdrag * proj.vel.z / mass
    };

    proj.vel = proj.vel + acc * dt;

    // --- TUTAJ DODAJEMY NOWY WSPÓŁCZYNNIK HAMOWANIA ---
    // Zmniejsza prędkość piłki w każdym kroku czasowym
    // Im mniejszy dampingFactor (bliżej 0), tym szybciej zwalnia.
    // DampingFactor = 1.0f oznacza brak hamowania.
    proj.vel = proj.vel * dampingFactor;
    // --------------------------------------------------

    proj.pos = proj.pos + proj.vel * dt;

    float sphereRadius = 0.5f; // Promień pocisku

    // Kolizja pocisku z ziemią
    if (proj.pos.y - sphereRadius <= 0.0f && proj.vel.y < 0.0f) {
        proj.pos.y = sphereRadius; // Odsuń piłkę na powierzchnię ziemi
        proj.vel.y = -proj.vel.y * restitution; // Odbicie
    }

    // Dodano warunek na zmianę pozycji dla trail, aby uwzględnić Z
    if (proj.trail.empty() || std::abs(proj.pos.x - proj.trail.back().x) > 1.0f || std::abs(proj.pos.y - proj.trail.back().y) > 1.0f || std::abs(proj.pos.z - proj.trail.back().z) > 1.0f) {
        proj.trail.push_back(proj.pos);
        if (proj.trail.size() > 100) proj.trail.erase(proj.trail.begin());
    }

    // Kolizje pocisku z klockami
    for (auto& block : blocks) {
        CollisionInfo colInfo = checkCollisionSphereAABB(proj.pos, sphereRadius, block);
        if (colInfo.collided) {
            // Rozwiązanie penetracji: odsuń piłkę
            // Dodajemy mały epsilon, aby upewnić się, że piłka jest poza obiektem
            proj.pos = proj.pos + colInfo.normal * (colInfo.penetrationDepth + 0.001f);

            // Odbicie prędkości pocisku
            // Obliczamy składową prędkości wzdłuż normalnej kolizji
            float velAlongNormal = proj.vel.dot(colInfo.normal);

            // Tylko jeśli obiekty się do siebie zbliżają
            if (velAlongNormal < 0) {
                Vec3 impulse = colInfo.normal * (velAlongNormal * (1.0f + restitution));
                proj.vel = proj.vel - impulse; // Odbicie
            }
        }
    }

    // Zatrzymanie ruchu, jeśli prędkość spadnie poniżej progu
    // Tylko jeśli piłka jest blisko ziemi (lub na niej)
    if (isRunning && proj.vel.length() < 0.2f && proj.pos.y < 1.0f) {
        proj.vel = { 0,0,0 }; // Ustaw prędkość na zero
        isRunning = false;  // Zatrzymuje symulację
    }
}


// Zmodyfikowana funkcja generująca wierzchołki sfery wraz z normalnymi i UV
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

std::vector<Vertex> sphereVertices;
std::vector<unsigned int> sphereIndices; // Dodano indeksy

// Wierzchołki dla standardowego sześcianu (1x1x1)
const float cubeVerticesData[] = {
    // Pozycje            // Normalne            // TexCoords
    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

    // Left face
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    // Right face
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     // Bottom face
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    // Top face
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

void generateSphereData(float radius, int segments) {
    sphereVertices.clear();
    sphereIndices.clear();

    for (int y = 0; y <= segments; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float xSegment = (float)x / segments;
            float ySegment = (float)y / segments;

            float xPos = radius * std::cos(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
            float yPos = radius * std::cos(ySegment * M_PI);
            float zPos = radius * std::sin(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);

            Vertex vert;
            vert.position = glm::vec3(xPos, yPos, zPos);
            vert.normal = glm::normalize(vert.position); // Normalne są takie same jak znormalizowane pozycje
            vert.texCoords = glm::vec2(xSegment, ySegment); // Współrzędne UV
            sphereVertices.push_back(vert);
        }
    }

    // Generowanie indeksów dla pasków trójkątów (Triangle Strips)
    for (int y = 0; y < segments; ++y) {
        for (int x = 0; x < segments; ++x) {
            int currentVertex = y * (segments + 1) + x;
            int nextRowVertex = (y + 1) * (segments + 1) + x;

            // Pierwszy trójkąt w kwadracie
            sphereIndices.push_back(currentVertex);
            sphereIndices.push_back(nextRowVertex);
            sphereIndices.push_back(currentVertex + 1);

            // Drugi trójkąt w kwadracie
            sphereIndices.push_back(currentVertex + 1);
            sphereIndices.push_back(nextRowVertex);
            sphereIndices.push_back(nextRowVertex + 1);
        }
    }
}


void initSphereVAO() {
    generateSphereData(0.5f, 32); // Zwiększono segmenty dla płynniejszej sfery

    glGenVertexArrays(1, &vaoSphere);
    glGenBuffers(1, &vboSphere);
    glGenBuffers(1, &eboSphere); // Generowanie EBO

    glBindVertexArray(vaoSphere);

    glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(Vertex), sphereVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSphere); // Bindowanie EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    // Pozycje (layout = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    // Normalne (layout = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // Współrzędne teksturowe (layout = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0); // Odwiązanie VAO
}

// Nowa funkcja do inicjalizacji VAO dla klocka
void initBlockVAO() {
    glGenVertexArrays(1, &vaoBlock);
    glGenBuffers(1, &vboBlock);

    glBindVertexArray(vaoBlock);
    glBindBuffer(GL_ARRAY_BUFFER, vboBlock);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerticesData), cubeVerticesData, GL_STATIC_DRAW);

    // Pozycje (layout = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normalne (layout = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Współrzędne teksturowe (layout = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}


// Funkcja ładowania tekstury, zwracająca jej ID
GLuint loadTexture(const char* path) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Ustawienia wrappingu
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Ustawienia filtrowania
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 4)
            format = GL_RGBA;
        else if (nrChannels == 3)
            format = GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        // W przypadku niepowodzenia, zwróć 0 (nieprawidłowe ID tekstury)
        glBindTexture(GL_TEXTURE_2D, 0); // Odwiąż wadliwą teksturę
        glDeleteTextures(1, &texID); // Usuń wygenerowane ID
        return 0;
    }
    stbi_image_free(data);
    return texID;
}


// Uogólniona funkcja renderująca obiekty
// Dodano parametr 'applyLighting'
void renderObject(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj_mat, const glm::vec4& color, GLuint currentTextureID, bool textured, bool applyLighting, GLuint vao, GLsizei elementCount, GLenum mode = GL_TRIANGLES) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, glm::value_ptr(proj_mat));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos)); // Dodano pozycję kamery dla oświetlenia

    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), textured ? 1 : 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "disableLighting"), applyLighting ? 0 : 1); // 0 = włącz oświetlenie, 1 = wyłącz oświetlenie

    if (textured && currentTextureID != 0) { // Sprawdzamy, czy tekstura jest poprawna
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTextureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, 0); // Jawnie odwiąż teksturę
        glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, glm::value_ptr(color));
    }

    glBindVertexArray(vao);
    if (vao == vaoSphere) {
        glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(mode, 0, elementCount);
    }
}

// Funkcje pomocnicze do renderowania konkretnych obiektów
// Zaktualizowano parametry dla renderSphere i renderBlock
void renderSphere(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj_mat, const glm::vec4& color, GLuint currentTextureID, bool textured = false, bool applyLighting = true) {
    renderObject(model, view, proj_mat, color, currentTextureID, textured, applyLighting, vaoSphere, sphereIndices.size(), GL_TRIANGLES);
}

void renderBlock(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj_mat, const glm::vec4& color, GLuint currentTextureID, bool textured = false, bool applyLighting = true) {
    renderObject(model, view, proj_mat, color, currentTextureID, textured, applyLighting, vaoBlock, 36); // Klocek ma 36 wierzchołków
}


void renderGround() {
    if (!vaoGround) {
        // Wierzchołki dla płaszczyzny, teraz z normalnymi i UV
        float groundVertices[] = {
            // Pozycje            // Normalne         // TexCoords (dostosowane do pokrycia większego obszaru)
            -200.0f, 0.0f, -200.0f,  0.0f, 1.0f, 0.0f,  0.0f, 200.0f,
             200.0f, 0.0f, -200.0f,  0.0f, 1.0f, 0.0f,  200.0f, 200.0f,
             200.0f, 0.0f,  200.0f,  0.0f, 1.0f, 0.0f,  200.0f, 0.0f,
            -200.0f, 0.0f,  200.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f
        };
        glGenVertexArrays(1, &vaoGround);
        glBindVertexArray(vaoGround);
        glGenBuffers(1, &vboGround);
        glBindBuffer(GL_ARRAY_BUFFER, vboGround);
        glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

        // Pozycje (layout = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Normalne (layout = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // Współrzędne teksturowe (layout = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    glm::mat4 model = glm::mat4(1.0f);
    // Ziemia z teksturą i oświetleniem
    // Przekazujemy 0 dla uColor, bo i tak będzie użyta tekstura
    renderObject(model, glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp), glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f), glm::vec4(0.0f), textures["textures/placeholder_ground.jpg"], true, true, vaoGround, 4, GL_TRIANGLE_FAN);
}

void renderScene() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Renderujemy wszystkie obiekty nieprzezroczyste najpierw
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Renderujemy ziemię
    renderGround();

    // Renderujemy klocki (nieprzezroczyste)
    for (auto& block : blocks) {
        glm::mat4 modelBlock = glm::translate(glm::mat4(1.0f), glm::vec3(block.pos.x, block.pos.y, block.pos.z));
        modelBlock = glm::scale(modelBlock, glm::vec3(block.size.x, block.size.y, block.size.z)); // Skalowanie klocka
        // Używamy tekstury przypisanej do klocka, włączamy teksturowanie, włączamy oświetlenie
        renderBlock(modelBlock, view, projection, glm::vec4(1.0f), block.textureID, true, true); // Kolor ustawiamy na biały, aby tekstura była widoczna
    }

    // Renderujemy pocisk (nieprzezroczysty, jeśli nie ma przezroczystości)
    glm::mat4 modelProj = glm::translate(glm::mat4(1.0f), glm::vec3(proj.pos.x, proj.pos.y, proj.pos.z));
    // Używamy textureIDProjectile, włączamy teksturowanie, włączamy oświetlenie
    renderSphere(modelProj, view, projection, glm::vec4(1.0f), textureIDProjectile, true, true); // Kolor ustawiamy na biały

    // Aktywacja blendingu dla przezroczystości
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Wyłącz zapis do bufora głębi dla przezroczystych obiektów, aby uniknąć artefaktów
    glDepthMask(GL_FALSE);

    // Renderujemy ślad pocisku (przezroczysty)
    for (auto& p : proj.trail) {
        glm::mat4 trailModel = glm::translate(glm::mat4(1.0f), glm::vec3(p.x, p.y, p.z));
        // Bez tekstury, wyłączone oświetlenie, przezroczystość
        renderSphere(trailModel, view, projection, glm::vec4(1.0f, 0.8f, 0.2f, 0.5f), 0, false, false);
    }

    // Przywróć zapis do bufora głębi
    glDepthMask(GL_TRUE);
    // Wyłącz blending po renderowaniu przezroczystych obiektów
    glDisable(GL_BLEND);
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error (" << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "):\n" << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram() {
    // Vertex Shader
    const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal; // Normalne
        layout(location = 2) in vec2 aTexCoords; // Współrzędne teksturowe

        uniform mat4 uProjection, uView, uModel;

        out vec2 TexCoords; // Przekazujemy TexCoords do Fragment Shadera
        out vec3 Normal;
        out vec3 FragPos;

        void main() {
            TexCoords = aTexCoords;
            FragPos = vec3(uModel * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(uModel))) * aNormal; // Transformacja normalnych
            gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        }
    )";

    // Fragment Shader
    const char* fs = R"(
        #version 330 core
        out vec4 FragColor;

        in vec2 TexCoords;
        in vec3 Normal;
        in vec3 FragPos;

        uniform vec4 uColor; // Kolor dla nietersturowanych obiektów (teraz z alfą)
        uniform sampler2D uTexture; // Tekstura
        uniform int useTexture; // Flaga do przełączania
        uniform int disableLighting; // Nowa flaga do wyłączania oświetlenia

        // Podstawowe oświetlenie
        uniform vec3 lightPos = vec3(0.0, 50.0, 50.0);
        uniform vec3 viewPos; // Pozycja kamery (widza)
        uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);

        void main() {
            vec3 lightingResult;

            if (disableLighting == 1) {
                // Jeśli oświetlenie wyłączone, ustawiamy na bazowy kolor (bez modyfikacji)
                lightingResult = vec3(1.0); // Brak wpływu oświetlenia na kolor
            } else {
                // Ambient
                vec3 ambient = 0.1 * lightColor;

                // Diffuse
                vec3 norm = normalize(Normal);
                vec3 lightDir = normalize(lightPos - FragPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * lightColor;

                // Specular (podstawowy)
                vec3 viewDir = normalize(viewPos - FragPos);
                vec3 reflectDir = reflect(-lightDir, norm);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32 to shininess
                vec3 specular = spec * lightColor * 0.5; // Mniejsza intensywność

                // Kombinacja wszystkich składników oświetlenia
                lightingResult = ambient + diffuse + specular;
            }

            if (useTexture == 1) {
                // Obiekty teksturowane
                vec4 texColor = texture(uTexture, TexCoords);
                FragColor = vec4(lightingResult * texColor.rgb, texColor.a);
            } else {
                // Obiekty kolorowe (bez tekstury)
                FragColor = vec4(lightingResult * uColor.rgb, uColor.a);
            }
        }
    )";

    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}


// Nowa funkcja do inicjalizacji wszystkich zasobów OpenGL
void initGL() {
    shaderProgram = createShaderProgram();
    initSphereVAO();
    initBlockVAO();

    // Załadowanie tekstur i zapisanie ich ID w mapie
    textures["textures/placeholder1.jpg"] = loadTexture("textures/placeholder1.jpg");
    textures["textures/placeholder2.jpg"] = loadTexture("textures/placeholder2.jpg");
    textures["textures/placeholder_ground.jpg"] = loadTexture("textures/placeholder_ground.jpg"); // Tekstura dla ziemi
    textureIDProjectile = loadTexture("textures/placeholder_ball.jpg"); // Tekstura dla pocisku

    // Upewnij się, że textures/placeholder_ball.jpg istnieje,
    // jeśli nie, możesz użyć domyślnej tekstury lub renderować bez niej.
    // Jeśli plik nie istnieje, loadTexture zwróci 0.
    if (textureIDProjectile == 0) {
        std::cerr << "Błąd: Nie załadowano tekstury dla pocisku. Pocisk może być renderowany jako kolor." << std::endl;
    }
}


int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rzut ukośny 3D", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Tryb swobodnej kamery domyślnie

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    initGL(); // Wywołujemy naszą nową funkcję inicjalizującą GL

    reset(); // Resetuje również klocki
    float lastTime = glfwGetTime();

    // Inicjalne ustawienie kursora na środek okna, gdy kamera jest w trybie swobodnym
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glfwSetCursorPos(window, width / 2, height / 2);
    lastX = width / 2;
    lastY = height / 2;

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window, deltaTime);
        glfwPollEvents();

        update(deltaTime); // Aktualizacja fizyki

        // Zapobiegamy przetwarzaniu wejścia myszy przez ImGui w trybie swobodnej kamery
        io.WantCaptureMouse = !freeCameraMode;
        io.WantCaptureKeyboard = !freeCameraMode;


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Sterowanie");
        ImGui::Text("Tryb kamery: %s (przelacz 'Z')", freeCameraMode ? "Swobodny" : "Statyczny");
        ImGui::SliderFloat("Predkosc poczatkowa", &velocity, 10.0f, 100.0f);
        ImGui::SliderFloat("Kat (stopnie)", &angle, 10.0f, 80.0f);
        ImGui::SliderFloat("Kierunek (obrot Y)", &launchYaw, -180.0f, 180.0f);
        ImGui::SliderFloat("Masa pocisku", &mass, 0.1f, 5.0f);
        ImGui::SliderFloat("Opor powietrza", &drag, 0.0f, 0.05f);
        ImGui::SliderFloat("Grawitacja", &gravity, 0.0f, 20.0f);
        ImGui::SliderFloat("Otracie (Restytucja)", &restitution, 0.0f, 1.0f);
        // TUTAJ ZNAJDUJE SIĘ SUWAK DO KONTROLI WSPÓŁCZYNNIKA HAMOWANIA
        ImGui::SliderFloat("Wspolczynnik Hamowania", &dampingFactor, 0.9f, 0.999f);

        if (ImGui::Button("Start")) { reset(); isRunning = true; }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) { reset(); }
        ImGui::Text("Pozycja pocisku: X=%.1f Y=%.1f Z=%.1f", proj.pos.x, proj.pos.y, proj.pos.z);
        ImGui::End();

        ImGui::Render();
        renderScene();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}