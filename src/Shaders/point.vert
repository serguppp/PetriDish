#version 130

in vec2 a_position; // Podstawowa pozycja (0,0) dla punktu

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform float u_pointSize;

// For fragment shader
flat out vec3 v_color_type_passthrough; 
uniform vec3 u_color_uniform; 


void main() {
    gl_Position = u_viewProjectionMatrix * u_modelMatrix * vec4(a_position, 0.0, 1.0);
    gl_PointSize = u_pointSize;
    v_color_type_passthrough = u_color_uniform;
}