#version 330 core

// Atrybuty wierzchołka 
layout (location = 0) in vec2 a_vertexPosition;

// Wyjścia do shadera fragmentów
out vec2 v_texCoord; 

void main() {
    gl_Position = vec4(a_vertexPosition, 0.0, 1.0); 
    v_texCoord = (a_vertexPosition + 1.0) * 0.5;
}
