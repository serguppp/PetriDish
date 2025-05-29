#version 330 core

// Wejścia z shadera wierzchołków
in vec2 v_texCoord; 

// Uniformy
uniform vec2 u_screenResolution;        // Rozdzielczość ekranu w pikselach
uniform vec2 u_lightScreenPosition;     // Pozycja światła w przestrzeni ekranu 
uniform vec3 u_glowEffectColor;         // Kolor poświaty
uniform float u_glowEffectRadius;       // Promień poświaty w pikselach
uniform float u_glowEffectIntensity;    // Intensywność poświaty

// Wyjście shadera
out vec4 out_FragColor;

void main() {
    // Współrzędne fragmentu w pikselach
    vec2 fragCoordPixels = v_texCoord * u_screenResolution;
    
    float distanceToLight = length(fragCoordPixels - u_lightScreenPosition);
    
    // Obliczanie intensywności poświaty na podstawie odległości
    float glow = 1.0 - smoothstep(0.0, u_glowEffectRadius, distanceToLight);
    glow *= u_glowEffectIntensity;
    
    out_FragColor = vec4(u_glowEffectColor * glow, glow); // Alfa zależna od intensywności
}
