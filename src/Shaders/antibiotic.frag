#version 330 core

// Uniformy
uniform vec4 u_effectColor;

// Wyjście shadera
out vec4 out_FragColor;

void main() {
    out_FragColor = u_effectColor;
}
