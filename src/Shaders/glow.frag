#version 130

in vec2 v_texCoord;
in vec2 v_fragPos_screen; // Pozycja fragmentu w pikselach ekranu

uniform vec3 u_glowColor;       // Kolor poświaty
uniform vec2 u_lightPos_screen; // Pozycja światła w pikselach ekranu
uniform float u_glowRadius;     // Promień poświaty w pikselach
uniform float u_glowIntensity;  // Intensywność poświaty

out vec4 FragColor;

void main() {
    float distanceToLight = length(v_fragPos_screen - u_lightPos_screen);

    float attenuation = 1.0 - smoothstep(0.0, u_glowRadius, distanceToLight);
    attenuation *= attenuation;

    float finalAlpha = attenuation * u_glowIntensity;
    finalAlpha = clamp(finalAlpha, 0.0, 1.0); 

    FragColor = vec4(u_glowColor, finalAlpha);
}