#pragma once
#include "glm/glm.hpp"
#include <GL/glew.h>
#include <string>
#include <map>
#include <vector> 
#include <iostream>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class ModelLoader {
public:
    bool loadOBJ(const char* path, std::vector<Vertex>& out_vertices);
};