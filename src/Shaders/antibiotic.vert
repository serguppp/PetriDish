#version 330 core

// Atrybuty wierzchołka 
layout (location = 0) in vec2 a_vertexPosition;

// Uniformy
uniform mat4 u_modelMatrix;             // Macierz modelu dla pozycji i skali efektu antybiotyku
uniform mat4 u_viewProjectionMatrix;    // Macierz widoku-projekcji

void main() {
    gl_Position = u_viewProjectionMatrix * u_modelMatrix * vec4(a_vertexPosition, 0.0, 1.0);
}
