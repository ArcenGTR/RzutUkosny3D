// default.vert
#version 330 core
layout(location = 0) in vec3 aPos;         // pozycja wierzcholka
layout(location = 1) in vec3 aNormal;      // normalna wierzcholka
layout(location = 2) in vec2 aTexCoords;   // wspolrzedne tekstury

uniform mat4 uProjection; // macierz rzutowania
uniform mat4 uView;       // macierz widoku kamery
uniform mat4 uModel;      // macierz modelu obiektu

out vec2 TexCoords; // przekazane wspolrzedne tekstury do fragment shadera
out vec3 Normal;    // przekazana normalna wierzcholka do fragment shadera
out vec3 FragPos;   // przekazana pozycja fragmentu do fragment shadera

void main() {
    TexCoords = aTexCoords; // przypisz wspolrzedne tekstury
    FragPos = vec3(uModel * vec4(aPos, 1.0)); // oblicz pozycje fragmentu w przestrzeni swiata
    Normal = mat3(transpose(inverse(uModel))) * aNormal; // transformuj normalna modelu
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0); // finalna pozycja wierzcholka na ekranie
}