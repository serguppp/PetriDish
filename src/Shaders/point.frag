#version 330 core

// Uniformy
uniform vec3 u_pointColor; 

// Wyjście shadera
out vec4 out_FragColor;

void main() {
    out_FragColor = vec4(u_pointColor, 1.0);
}
