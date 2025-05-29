// rzut_ukosny.cpp
// Компиляция: g++ rzut_ukosny.cpp -lglfw -ldl -lGL -lX11 -lpthread -lXrandr -lXi -lXxf86vm -lXcursor -o rzut
#define M_PI 3.14159265358979323846f

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vec3 {
    float x, y, z;
    Vec3 operator+(const Vec3& b) const { return { x + b.x, y + b.y, z + b.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
};

struct Projectile {
    Vec3 pos, vel;
    std::vector<Vec3> trail;
};

Projectile proj;
bool isRunning = false;

float gravity = 9.81f;
float velocity = 50.0f, angle = 45.0f, mass = 1.0f, drag = 0.01f;
float restitution = 0.6f;
float rotX = 0.0f, rotY = 45.0f, zoom = 100.0f;
float camXOffset = 0.0f, camYOffset = 20.0f, camZOffset = 0.0f;


float toRadians(float degrees) { return degrees * 3.1415926f / 180.0f; }

void reset() {
    float rad = toRadians(angle);
    proj.pos = { 0, 0.5f, 0 };
    proj.vel = { velocity * std::cos(rad), velocity * std::sin(rad), 0 };
    proj.trail.clear();
    isRunning = false;
}

void update(float dt) {
    if (!isRunning) return;

    float speed = std::sqrt(proj.vel.x * proj.vel.x + proj.vel.y * proj.vel.y + proj.vel.z * proj.vel.z);
    float fdrag = drag * speed;
    Vec3 acc = {
        -fdrag * proj.vel.x / mass,
        -gravity - fdrag * proj.vel.y / mass,
        -fdrag * proj.vel.z / mass
    };

    proj.vel = proj.vel + acc * dt;
    proj.pos = proj.pos + proj.vel * dt;

    if (proj.trail.empty() || std::abs(proj.pos.x - proj.trail.back().x) > 1.0f || std::abs(proj.pos.y - proj.trail.back().y) > 1.0f) {
        proj.trail.push_back(proj.pos);
        if (proj.trail.size() > 100) proj.trail.erase(proj.trail.begin());
    }

    if (proj.pos.y <= 0.0f && proj.vel.y < 0.0f) {
        proj.pos.y = 0.0f;
        proj.vel.y = -proj.vel.y * restitution;

        if (std::abs(proj.vel.y) < 0.5f) {
            proj.vel.y = 0;
            isRunning = false;
        }
    }
}

GLuint vaoGround = 0, vaoSphere = 0, vboGround = 0, vboSphere = 0;
GLuint shaderProgram = 0;

std::vector<float> createSphereVertices(float radius, int segments) {
    std::vector<float> vertices;
    for (int y = 0; y <= segments; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float xSegment = (float)x / segments;
            float ySegment = (float)y / segments;
            float xPos = radius * std::cos(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
            float yPos = radius * std::cos(ySegment * M_PI);
            float zPos = radius * std::sin(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
        }
    }
    return vertices;
}

void initSphereVAO() {
    std::vector<float> sphereVertices = createSphereVertices(0.5f, 16);
    glGenVertexArrays(1, &vaoSphere);
    glGenBuffers(1, &vboSphere);
    glBindVertexArray(vaoSphere);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void renderSphere(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj, const glm::vec3& color) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, glm::value_ptr(color));

    glBindVertexArray(vaoSphere);
    glDrawArrays(GL_POINTS, 0, 289); // 17x17 grid assumed for 16 segments
}

void renderGround() {
    if (!vaoGround) {
        float groundVertices[] = {
            -200, 0, -200,
             200, 0, -200,
             200, 0,  200,
            -200, 0,  200
        };
        glGenVertexArrays(1, &vaoGround);
        glBindVertexArray(vaoGround);
        glGenBuffers(1, &vboGround);
        glBindBuffer(GL_ARRAY_BUFFER, vboGround);
        glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glm::mat4 model = glm::mat4(1.0f);
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 0.2f, 0.5f, 0.2f);
    glBindVertexArray(vaoGround);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void renderScene() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
    float camX = zoom * sin(toRadians(rotY)) * cos(toRadians(rotX));
    float camY = zoom * sin(toRadians(rotX));
    float camZ = zoom * cos(toRadians(rotY)) * cos(toRadians(rotX));
    glm::mat4 view = glm::lookAt(glm::vec3(camX, camY + 20, camZ), glm::vec3(0, 10, 0), glm::vec3(0, 1, 0));

    renderGround();

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(proj.pos.x, proj.pos.y, proj.pos.z));
    renderSphere(model, view, projection, glm::vec3(1.0f, 0.3f, 0.3f));

    for (auto& p : proj.trail) {
        glm::mat4 trailModel = glm::translate(glm::mat4(1.0f), glm::vec3(p.x, p.y, p.z));
        renderSphere(trailModel, view, projection, glm::vec3(1.0f, 0.8f, 0.2f));
    }
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    return shader;
}

GLuint createShaderProgram() {
    const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 uProjection, uView, uModel;
        void main() {
            gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        }
    )";

    const char* fs = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main() {
            FragColor = vec4(uColor, 1.0);
        }
    )";

    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rzut ukośny 3D", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    shaderProgram = createShaderProgram();
    initSphereVAO();
    reset();
    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        glfwPollEvents();
        update(deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Sterowanie");
        ImGui::SliderFloat("Predkosc poczatkowa", &velocity, 10.0f, 100.0f);
        ImGui::SliderFloat("Kat (stopnie)", &angle, 10.0f, 80.0f);
        ImGui::SliderFloat("Masa", &mass, 0.1f, 5.0f);
        ImGui::SliderFloat("Opor powietrza", &drag, 0.0f, 0.05f);
        ImGui::SliderFloat("Grawitacja", &gravity, 0.0f, 20.0f);
        ImGui::SliderFloat("Otracie (Restytucja)", &restitution, 0.0f, 1.0f);
        ImGui::SliderFloat("Obrot X", &rotX, -90.0f, 90.0f);
        ImGui::SliderFloat("Obrot Y", &rotY, -180.0f, 360.0f);
        ImGui::SliderFloat("Zoom", &zoom, 50.0f, 200.0f);

        if (ImGui::Button("Start")) { reset(); isRunning = true; }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) { reset(); }
        ImGui::Text("Pozycja: X=%.1f Y=%.1f Z=%.1f", proj.pos.x, proj.pos.y, proj.pos.z);
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