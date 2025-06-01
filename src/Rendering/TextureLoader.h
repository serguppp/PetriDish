#pragma once

#include <GL/glew.h>
#include <string>
#include <map>
#include <vector> 
#include <iostream>

class TextureLoader {
public:
    static GLuint loadTexture(const char* filename);
};