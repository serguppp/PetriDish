#version 330 core

// Atrybuty wierzchołka 
layout (location = 0) in vec2 a_vertexPosition;

// Uniformy
uniform mat4 u_modelMatrix;             // Macierz modelu 
uniform mat4 u_viewProjectionMatrix;    // Macierz widoku-projekcji
uniform float u_renderPointSize;        // Rozmiar punktu

void main() {
    gl_Position = u_viewProjectionMatrix * u_modelMatrix * vec4(a_vertexPosition, 0.0, 1.0);
    gl_PointSize = u_renderPointSize;
}
