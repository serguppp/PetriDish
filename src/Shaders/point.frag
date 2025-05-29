#version 130

flat in vec3 v_color_type_passthrough; 
out vec4 FragColor;

void main() {
    FragColor = vec4(v_color_type_passthrough, 1.0);
}