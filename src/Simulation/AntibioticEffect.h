#pragma once

#include <glm/glm.hpp>
#include <vector>

struct AntibioticEffect {
    glm::vec2 worldPosition;
    float strength;
    float radius;
    float timeApplied; 
    float maxLifetime;
};