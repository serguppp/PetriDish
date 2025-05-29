#include <vector>
#include <string>
#include "glm/glm.hpp"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

bool loadOBJ(const char* path, std::vector<Vertex>& out_vertices);
