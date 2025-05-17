#version 130

in vec2 v_localPos;
in vec2 v_worldPos_no_mvp;

uniform int u_bacteriaType;
uniform float u_health;
uniform float u_time; 

out vec4 out_FragColor;

// Proste funkcje szumu 
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
    vec3 baseColor;
    float proceduralPattern = 0.0;
    float alpha = (u_health > 0.05) ? (0.2 + u_health * 0.8) : (u_health * 2.0); 


    if (u_bacteriaType == 0) { // Cocci
        baseColor = vec3(0.9, 0.4, 0.4);
        // Wzór: pulsowanie jasności oparte na odległości od środka i czasie
        float dist_from_local_center = length(v_localPos);
        proceduralPattern = (sin(dist_from_local_center * 5.0 - u_time * 2.0) + 1.0) / 2.0;
        baseColor = mix(baseColor * 0.7, baseColor * 1.1, proceduralPattern);

    } else if (u_bacteriaType == 1) { // Diplococcus
        baseColor = vec3(0.4, 0.9, 0.4);
        // Wzór: dwa pulsujące centra 
        // v_localPos.x może być np. od -1.6 do 1.6 dla Diplococcus
        float lobe1 = smoothstep(0.5, 0.0, length(v_localPos - vec2(-0.8, 0.0)));
        float lobe2 = smoothstep(0.5, 0.0, length(v_localPos - vec2(0.8, 0.0)));
        float pulse = (sin(u_time + v_localPos.x * 2.0) + 1.0) / 2.0;
        baseColor *= (0.6 + 0.4 * max(lobe1, lobe2) * pulse);

    } else if (u_bacteriaType == 2) { // Staphylococci
        baseColor = vec3(0.4, 0.4, 0.9);
        // Wzór: plamki z szumu
        float spots = noise(v_localPos, 8.0 + sin(u_time)*2.0); // Skala szumu animowana
        spots = pow(spots, 3.0) * 1.5; // Wzmocnienie kontrastu
        baseColor = mix(baseColor * 0.6, baseColor * 1.2, spots);

    } else if (u_bacteriaType == 3) { // Bacillus
        baseColor = vec3(0.8, 0.6, 0.2);
        // Wzór: paski w poprzek 
        proceduralPattern = (cos(v_localPos.x * 15.0 + u_time) + 1.0) / 2.0;
        baseColor = mix(baseColor * 0.7, baseColor * 1.0, proceduralPattern);
        // Dodatkowe przyciemnienie na brzegach dla efektu 3D
        float edgeFactor = 1.0 - pow(abs(v_localPos.x / 1.5), 4.0); 
        edgeFactor *= (1.0 - pow(abs(v_localPos.y / 0.4), 4.0));
        baseColor *= (0.5 + 0.5 * edgeFactor);

    } else {
        baseColor = vec3(0.7, 0.7, 0.7); 
    }

    baseColor *= (0.3 + 0.7 * u_health); //  wpływ hp na jasność
    out_FragColor = vec4(baseColor, alpha);
}