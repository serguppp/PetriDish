#version 130 // GLSL 1.30

in vec2 a_position;    // Lokalna pozycja wierzchołka z circuit

uniform mat4 u_mvp;          // Macierz mvp
uniform vec2 u_worldPosition; // Pozycja środka bakterii w świecie
uniform float u_scale;       // Skala bakterii

// dane przekazywane do fragment shadera
out vec2 v_localPos;      // Lokalna pozycja wierzchołka 
out vec2 v_worldPos_no_mvp; // Pozycja w świecie przed transformacją MVP


void main() {
    vec2 scaled_local_pos = a_position * u_scale;
    vec2 final_world_pos = scaled_local_pos + u_worldPosition;
    gl_Position = u_mvp * vec4(final_world_pos, 0.0, 1.0);

    v_localPos = a_position; // Przekaż oryginalne lokalne współrzędne
    v_worldPos_no_mvp = final_world_pos;
}