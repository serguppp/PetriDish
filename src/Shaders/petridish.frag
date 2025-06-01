#version 330 core
out vec4 FragColor;

in vec3 v_fragWorldPosition;
in vec3 v_normalWorld;
in vec2 v_texCoords;

uniform vec3 u_objectColor;
uniform float u_objectAlpha;

uniform vec3 u_lightPosWorld;
uniform vec3 u_lightColor;
uniform float u_lightRange; 
uniform vec3 u_ambientColor;
uniform vec3 u_cameraPositionWorld;

uniform sampler2D uTextureSampler; 

void main() {
    // Oswietlenie otoczenia
    vec3 ambient = u_ambientColor;

    // Swiatlo  zalezne od kąta padania światła na powierzchnię.
    vec3 norm = normalize(v_normalWorld);
    vec3 lightDir = normalize(u_lightPosWorld - v_fragWorldPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Tlumienie swiatla wraz z odlegloscia
    float distanceToLight = length(u_lightPosWorld - v_fragWorldPosition);
    float attenuation = 1.0 / (1.0 + 0.01 * distanceToLight + 0.001 * distanceToLight * distanceToLight);
    attenuation = clamp(attenuation, 0.0, 1.0);
    
    // Czynnik odpowiedzialny za intensywność światła w zależności od zasięgu
    float lightRangeFactor = 1.0 - smoothstep(u_lightRange * 0.2, u_lightRange, distanceToLight); 
    lightRangeFactor = clamp(lightRangeFactor, 0.0, 1.0);

    // Kolor tekstury
    vec4 texColor = texture(uTextureSampler, v_texCoords);
    vec3 objectBaseColor = u_objectColor * texColor.rgb;

    vec3 diffuse = u_lightColor * diff * objectBaseColor * attenuation * lightRangeFactor;

    // Blinn-Phong
    vec3 viewDir = normalize(u_cameraPositionWorld - v_fragWorldPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); 
    vec3 specular = u_lightColor * spec * vec3(0.5) * attenuation * lightRangeFactor; 
                                                                  
    vec3 phongColor = ambient * objectBaseColor + diffuse + specular; 
                                                            
    FragColor = vec4(phongColor, u_objectAlpha);
}