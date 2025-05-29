#version 330 core

// Wejścia z shadera wierzchołków
in vec3 v_fragWorldPosition;
in vec3 v_normalWorld;
in float v_health;
flat in int v_bacteriaTypeOut; 
in vec2 v_localPosition; 

// Uniformy
uniform float u_time;                   // Czas globalny
uniform vec3 u_lightPositionWorld;      // Pozycja światła w przestrzeni świata
uniform vec3 u_lightColor;              // Kolor światła
uniform vec3 u_ambientColor;            // Kolor otoczenia
uniform float u_lightRange;             // Zasięg światła

// Wyjście shadera
out vec4 out_FragColor;

// Funkcja do generowania prostego szumu proceduralnego
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise(vec2 st, float scale) {
    vec2 i = floor(st * scale);
    vec2 f = fract(st * scale);
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f); 
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

void main() {
    vec3 patternedBaseColor;

    if (v_bacteriaTypeOut == 0) { // Cocci
        patternedBaseColor = vec3(0.9, 0.4, 0.4);
        float dist_from_local_center = length(v_localPosition);
        float localProceduralPattern = (sin(dist_from_local_center * 5.0 - u_time * 2.0) + 1.0) / 2.0;
        patternedBaseColor = mix(patternedBaseColor * 0.7, patternedBaseColor * 1.1, localProceduralPattern);
   
    } else if (v_bacteriaTypeOut == 1) { // Diplococcus
        patternedBaseColor = vec3(0.4, 0.9, 0.4);
        float lobeIntensity = 0.0;

        float distLobe1 = length(v_localPosition - vec2(-0.5, 0.0)); 
        lobeIntensity = max(lobeIntensity, 1.0 - smoothstep(0.0, 0.5, distLobe1)); 
        float distLobe2 = length(v_localPosition - vec2(0.5, 0.0));  

        lobeIntensity = max(lobeIntensity, 1.0 - smoothstep(0.0, 0.5, distLobe2));
        
        float pulse = (sin(u_time + v_localPosition.x * 2.0) + 1.0) / 2.0;
        patternedBaseColor *= (0.6 + 0.4 * lobeIntensity * pulse);

    } else if (v_bacteriaTypeOut == 2) { // Staphylococci
        patternedBaseColor = vec3(0.4, 0.4, 0.9);
        float spots = noise(v_localPosition, 8.0 + sin(u_time)*2.0);
        spots = pow(spots, 3.0) * 1.5;
        patternedBaseColor = mix(patternedBaseColor * 0.6, patternedBaseColor * 1.2, spots);

    } else if (v_bacteriaTypeOut == 3) { //Bacillus
        patternedBaseColor = vec3(0.8, 0.6, 0.2);
        float localProceduralPattern = (cos(v_localPosition.x * 15.0 + u_time) + 1.0) / 2.0;
        patternedBaseColor = mix(patternedBaseColor * 0.7, patternedBaseColor * 1.0, localProceduralPattern);
    
        float edgeFactorX = 1.0 - pow(abs(v_localPosition.x / 1.5), 4.0); 
        float edgeFactorY = 1.0 - pow(abs(v_localPosition.y / 0.4), 4.0); 
        float edgeFactor = clamp(edgeFactorX * edgeFactorY, 0.0, 1.0);

        float pulse = (sin(u_time + v_localPosition.x * 2.0) + 1.0) / 2.0;
        patternedBaseColor *= (0.5 + 0.5 * edgeFactor * pulse);

    } else {
        patternedBaseColor = vec3(0.7, 0.7, 0.7); 
    }

    // Kolor w zależności od zdrowia
    vec3 objectColor = patternedBaseColor * (0.3 + 0.7 * v_health);
    float alpha = (v_health > 0.05) ? (0.2 + v_health * 0.8) : (v_health / 0.05 * 0.3);
    alpha = clamp(alpha, 0.0, 1.0);

    // Obliczanie oświetlenia
    vec3 norm = normalize(v_normalWorld);
    vec3 lightDir = normalize(u_lightPositionWorld - v_fragWorldPosition);
    
    float distanceToLight = length(u_lightPositionWorld - v_fragWorldPosition);
    float attenuation = 1.0;

    if (u_lightRange > 0.001) {
        // Zanikanie światła
        attenuation = 1.0 - smoothstep(u_lightRange * 0.5, u_lightRange, distanceToLight); 
    } else {
        attenuation = 0.0; // Jeśli zasięg jest zerowy lub bliski zeru, brak światła
    }
    
    vec3 ambient = u_ambientColor * objectColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_lightColor * diff * objectColor;

    vec3 finalLitColor = (ambient + diffuse) * attenuation;
    
    out_FragColor = vec4(finalLitColor, alpha);
}
