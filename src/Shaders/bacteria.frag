#version 130

// Dane wejściowe z Vertex Shadera
in vec2 v_localPos;
in vec3 v_fragPos_world;
in vec3 v_normal_world;

// Uniformy podstawowe
uniform int u_bacteriaType;
uniform float u_health;
uniform float u_time;

// Uniformy dla oświetlenia
uniform vec3 u_lightPosition_world;
uniform vec3 u_lightColor;
uniform vec3 u_ambientColor;
uniform float u_lightRange; 

out vec4 FragColor;

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

    if (u_bacteriaType == 0) { // Cocci
        patternedBaseColor = vec3(0.9, 0.4, 0.4);
        float dist_from_local_center = length(v_localPos);
        float localProceduralPattern = (sin(dist_from_local_center * 5.0 - u_time * 2.0) + 1.0) / 2.0;
        patternedBaseColor = mix(patternedBaseColor * 0.7, patternedBaseColor * 1.1, localProceduralPattern);
    } else if (u_bacteriaType == 1) { // Diplococcus
        patternedBaseColor = vec3(0.4, 0.9, 0.4);
        float lobe1 = smoothstep(0.5, 0.0, length(v_localPos - vec2(-0.6, 0.0)));
        float lobe2 = smoothstep(0.5, 0.0, length(v_localPos - vec2(0.6, 0.0)));
        float pulse = (sin(u_time + v_localPos.x * 2.0) + 1.0) / 2.0;
        patternedBaseColor *= (0.6 + 0.4 * max(lobe1, lobe2) * pulse);
    } else if (u_bacteriaType == 2) { // Staphylococci
        patternedBaseColor = vec3(0.4, 0.4, 0.9);
        float spots = noise(v_localPos, 8.0 + sin(u_time)*2.0);
        spots = pow(spots, 3.0) * 1.5;
        patternedBaseColor = mix(patternedBaseColor * 0.6, patternedBaseColor * 1.2, spots);
    } else if (u_bacteriaType == 3) { //Bacillus
        patternedBaseColor = vec3(0.8, 0.6, 0.2);
        float localProceduralPattern = (cos(v_localPos.x * 15.0 + u_time) + 1.0) / 2.0;
        patternedBaseColor = mix(patternedBaseColor * 0.7, patternedBaseColor * 1.0, localProceduralPattern);
        
        float edgeFactorX = 1.0 - pow(abs(v_localPos.x / 1.5), 4.0);
        float edgeFactorY = 1.0 - pow(abs(v_localPos.y / 0.4), 4.0);
        float edgeFactor = clamp(edgeFactorX * edgeFactorY, 0.0, 1.0);
        patternedBaseColor *= (0.5 + 0.5 * edgeFactor);
    } else {
        patternedBaseColor = vec3(0.7, 0.7, 0.7);
    }

    vec3 objectColor = patternedBaseColor * (0.3 + 0.7 * u_health);
    float alpha = (u_health > 0.05) ? (0.2 + u_health * 0.8) : (u_health / 0.05 * 0.3);
    alpha = clamp(alpha, 0.0, 1.0);

    // Obliczanie oświetlenia
    vec3 norm = normalize(v_normal_world);
    vec3 lightDir = normalize(u_lightPosition_world - v_fragPos_world);
    
    float distanceToLight = length(u_lightPosition_world - v_fragPos_world);
    float attenuation = 1.0;

    if (u_lightRange > 0.001) {
        float normalizedDistance = clamp(distanceToLight / u_lightRange, 0.0, 1.0);
        attenuation = 1.0 - normalizedDistance * 0.95;
    } else {
        attenuation = 0.0;
    }
    
    vec3 ambient = u_ambientColor * objectColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_lightColor * diff * objectColor;

    vec3 finalLitColor = (ambient + diffuse) * attenuation;
    
    FragColor = vec4(finalLitColor, alpha);
}