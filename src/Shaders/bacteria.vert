#version 330 core

// Atrybuty wierzchołka
layout (location = 0) in vec2 a_vertexLocalPosition; // Lokalna pozycja wierzchołka modelu bakterii

// Uniformy
uniform mat4 u_viewProjectionMatrix;    // Macierz widoku-projekcji
uniform vec2 u_instanceWorldPosition;   // Pozycja instancji bakterii w świecie
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
    // Transformacja pozycji wierzchołka bakterii do przestrzeni świata
    vec2 worldPos = u_instanceWorldPosition + (a_vertexLocalPosition * u_instanceScale);
    
    // Ustawienie pozycji w przestrzeni 
    gl_Position = u_viewProjectionMatrix * vec4(worldPos, 0.0, 1.0);

    // Przekazanie danych do shadera fragmentów
    v_fragWorldPosition = vec3(worldPos, 0.0); 
    v_normalWorld = vec3(0.0, 0.0, 1.0); 
    v_health = u_bacteriaHealth;
    v_bacteriaTypeOut = u_bacteriaType;
    v_localPosition = a_vertexLocalPosition;
}
