// default.frag
#version 330 core
out vec4 FragColor; // koncowy kolor piksela

in vec2 TexCoords; // interpolowane wspolrzedne tekstury z vertex shadera
in vec3 Normal;    // interpolowana normalna z vertex shadera
in vec3 FragPos;   // interpolowana pozycja fragmentu z vertex shadera

uniform vec4 uColor;          // kolor bazowy jesli brak tekstury
uniform sampler2D uTexture;   // obiekt tekstury
uniform int useTexture;       // flaga czy uzyc tekstury (1 tak 0 nie)
uniform int disableLighting;  // flaga czy wylaczyc oswietlenie (1 tak 0 nie)

uniform vec3 lightPos = vec3(0.0, 50.0, 50.0); // pozycja zrodla swiatla
uniform vec3 viewPos; // pozycja kamery widza
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0); // kolor swiatla

void main() {
    vec3 lightingResult; // wynik oswietlenia

    if (disableLighting == 1) { // jesli oswietlenie wylaczone
        lightingResult = vec3(1.0); // ustaw kolor na bialy (pelne swiatlo)
    } else { // jesli oswietlenie wlaczone
        vec3 ambient = 0.1 * lightColor; // skladowa ambient (swiatlo otoczenia)

        vec3 norm = normalize(Normal); // znormalizowana normalna powierzchni
        vec3 lightDir = normalize(lightPos - FragPos); // wektor od fragmentu do swiatla
        float diff = max(dot(norm, lightDir), 0.0); // skladowa diffuse (rozproszona)
        vec3 diffuse = diff * lightColor; // kolor rozproszony

        vec3 viewDir = normalize(viewPos - FragPos); // wektor od fragmentu do kamery
        vec3 reflectDir = reflect(-lightDir, norm); // odbity wektor swiatla
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // skladowa specular (lustrzana)
        vec3 specular = spec * lightColor * 0.5; // kolor lustrzany

        lightingResult = ambient + diffuse + specular; // suma skladowych oswietlenia
    }

    if (useTexture == 1) { // jesli uzywamy tekstury
        vec4 texColor = texture(uTexture, TexCoords); // pobierz kolor z tekstury
        FragColor = vec4(lightingResult * texColor.rgb, texColor.a); // koncowy kolor z tekstura i oswietleniem
    } else { // jesli uzywamy koloru
        FragColor = vec4(lightingResult * uColor.rgb, uColor.a); // koncowy kolor z uColor i oswietleniem
    }
}