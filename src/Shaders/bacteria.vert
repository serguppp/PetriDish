#version 130

// Atrybut wierzchołka z VBO - lokalne współrzędne modelu
in vec2 a_position;

// Uniformy 
uniform mat4 u_mvp;             // Macierz Model-View-Projection
uniform vec2 u_worldPosition;   // Centralna pozycja bakterii w przestrzeni świata
uniform float u_scale;          // Skala bakterii

// Wyjścia do Fragment Shadera 
out vec2 v_localPos;        // Lokalne współrzędne dla wzorów proceduralnych
out vec3 v_fragPos_world;   // Pozycja fragmentu w przestrzeni świata dla oświetlenia
out vec3 v_normal_world;    // Wektor normalny w przestrzeni świata dla oświetlenia

void main() {
    // v_localPos to oryginalne współrzędne a_position,
    v_localPos = a_position;

    // Obliczanie pozycji wierzchołka w przestrzeni świata
    vec2 scaled_model_pos = a_position * u_scale;
    vec4 world_pos_of_vertex = vec4(u_worldPosition + scaled_model_pos, 0.0, 1.0); 

    // Przekazanie pozycji w świecie do fragment shadera
    v_fragPos_world = world_pos_of_vertex.xyz;

    // Dla płaskich bakterii 2D na płaszczyźnie XY, normalna jest zawsze (0,0,1)
    // skierowana w stronę dodatniej osi Z (w stronę kamery)
    v_normal_world = vec3(0.0, 0.0, 1.0);

    // Końcowa pozycja wierzchołka dla rasteryzacji
    gl_Position = u_mvp * world_pos_of_vertex;
}