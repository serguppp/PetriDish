#version 330 core

// Atrybuty wierzchołka
layout (location = 0) in vec2 a_vertexLocalPosition; // Lokalna pozycja wierzchołka modelu bakterii

// Uniformy
uniform mat4 u_viewProjectionMatrix;    // Macierz widoku-projekcji
uniform vec3 u_instanceWorldPosition;   // Pozycja instancji bakterii w świecie (X, Y, Z-index) // ZMIENIONO na vec3
uniform float u_instanceScale;          // Skala instancji bakterii
uniform float u_bacteriaHealth;         // Kondycja bakterii 
uniform int u_bacteriaType;             // Typ bakterii 
uniform float u_time;                   // Czas globalny dla animacji

// Wyjścia do shadera fragmentów
out vec3 v_fragWorldPosition; 
out vec3 v_normalWorld;         
out float v_health;
flat out int v_bacteriaTypeOut; 
out vec2 v_localPosition; 

void main() {
    vec3 worldPosWithZ = vec3(
        u_instanceWorldPosition.x + (a_vertexLocalPosition.x * u_instanceScale),
        u_instanceWorldPosition.y + (a_vertexLocalPosition.y * u_instanceScale),
        u_instanceWorldPosition.z  
    );
    
    // Ustawienie pozycji w przestrzeni wynikowej 
    gl_Position = u_viewProjectionMatrix * vec4(worldPosWithZ, 1.0);

    // Przekazanie danych do shadera fragmentów
    v_fragWorldPosition = worldPosWithZ; 
    v_normalWorld = vec3(0.0, 0.0, 1.0); 
    v_health = u_bacteriaHealth;
    v_bacteriaTypeOut = u_bacteriaType;
    v_localPosition = a_vertexLocalPosition;
}