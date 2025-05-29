#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat3 u_normalMatrix; 

out vec3 Normal;
out vec3 FragPosWorld;

void main() {
    FragPosWorld = vec3(u_modelMatrix * vec4(aPos, 1.0));
    Normal = u_normalMatrix * aNormal;
    gl_Position = u_viewProjectionMatrix * vec4(FragPosWorld, 1.0);
}