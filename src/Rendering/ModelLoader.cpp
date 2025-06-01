#include "ModelLoader.h"
#include <fstream>
#include <sstream>
#include <iostream> 

bool ModelLoader::loadOBJ(const char* path, std::vector<Vertex>& out_vertices) {
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_texcoords;
    std::vector<glm::vec3> temp_normals;

    std::vector<unsigned int> positionIndices, uvIndices, normalIndices;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR::MODEL_LOADER::Could not open file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 position;
            ss >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        } else if (prefix == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            uv.y = 1.0f - uv.y;
            temp_texcoords.push_back(uv);
        } else if (prefix == "vn") {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        } else if (prefix == "f") {
            std::string vertex1, vertex2, vertex3;
            unsigned int posIdx[3], uvIdx[3], normIdx[3];

            for (int i = 0; i < 3; ++i) {
                std::string part;
                ss >> part;
                std::stringstream face_ss(part);
                std::string segment;
                
                std::getline(face_ss, segment, '/');
                posIdx[i] = std::stoi(segment);
                
                if(std::getline(face_ss, segment, '/')) {
                    if (!segment.empty()) uvIdx[i] = std::stoi(segment);
                    else uvIdx[i] = 0; 
                } else {
                    uvIdx[i] = 0;
                }

                if(std::getline(face_ss, segment, '/')) { 
                     if (!segment.empty()) normIdx[i] = std::stoi(segment);
                     else normIdx[i] = 0; 
                } else {
                    normIdx[i] = 0;
                }
            }
            
            for(int i=0; i<3; ++i) {
                positionIndices.push_back(posIdx[i]);
                uvIndices.push_back(uvIdx[i]);
                normalIndices.push_back(normIdx[i]);
            }
        }
    }
    file.close();

    for (unsigned int i = 0; i < positionIndices.size(); i++) {
        Vertex vertex;
        vertex.Position = temp_positions[positionIndices[i] - 1];
        if (!temp_texcoords.empty() && uvIndices[i] > 0) {
             vertex.TexCoords = temp_texcoords[uvIndices[i] - 1];
        } else {
             vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        if (!temp_normals.empty() && normalIndices[i] > 0) {
            vertex.Normal = temp_normals[normalIndices[i] - 1];
        } else {
            vertex.Normal = glm::vec3(0.0f, 0.0f, 1.0f); 
        }
        out_vertices.push_back(vertex);
    }
    return true;
}