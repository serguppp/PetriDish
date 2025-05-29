#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPosWorld;

uniform vec3 u_objectColor;
uniform float u_objectAlpha;

uniform vec3 u_lightPosWorld;
uniform vec3 u_lightColor;
uniform vec3 u_ambientColor;
uniform vec3 u_cameraPositionWorld;
uniform float u_lightRange;

void main() {
    // Oświetlenie 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_lightPosWorld - FragPosWorld);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_lightColor;

    vec3 viewDir = normalize(u_cameraPositionWorld - FragPosWorld);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); 
    vec3 specular = spec * u_lightColor; 

    float distanceToLight = length(u_lightPosWorld - FragPosWorld);
    float attenuation = clamp(1.0 - pow(distanceToLight / u_lightRange, 2.0), 0.0, 1.0);
    attenuation *= attenuation;

    vec3 resultColor = (u_ambientColor + diffuse * attenuation + specular * attenuation) * u_objectColor;
    
    FragColor = vec4(resultColor, u_objectAlpha); 
}