#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoords; 

out vec3 v_fragWorldPosition;
out vec3 v_normalWorld;
out vec2 v_texCoords;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat3 u_normalMatrix;

void main() {
    v_fragWorldPosition = vec3(u_modelMatrix * vec4(a_pos, 1.0));
    v_normalWorld = normalize(u_normalMatrix * a_normal);
    v_texCoords = a_texCoords; 
    gl_Position = u_viewProjectionMatrix * vec4(v_fragWorldPosition, 1.0);
}