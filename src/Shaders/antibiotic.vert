#version 130

in vec2 a_vertex; // Lokalna pozycja wierzchołka

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;

void main() {
    gl_Position = u_viewProjectionMatrix * u_modelMatrix * vec4(a_vertex, 0.0, 1.0);
}