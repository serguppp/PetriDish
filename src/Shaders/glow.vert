#version 130

in vec2 a_position;

uniform mat4 u_mvp;
uniform vec2 u_lightPos_screen; 
uniform vec2 u_resolution;     

out vec2 v_texCoord; 
out vec2 v_fragPos_screen; 

void main() {
    gl_Position = u_mvp * vec4(a_position, 0.0, 1.0);
    v_texCoord = a_position * 0.5 + 0.5; 
    v_fragPos_screen = (gl_Position.xy / gl_Position.w) * 0.5 + 0.5; 
    v_fragPos_screen *= u_resolution;
}