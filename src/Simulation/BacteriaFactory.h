#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include "IBacteria.h"
#include "Bacteria.h"
#include <memory>

class BacteriaFactory {
public:
    static std::unique_ptr<IBacteria> create(BacteriaType type, int x, int y){
        glm::vec4 position(x, y, 0.0f, 1.0f);
        return std::make_unique<Bacteria>(position, type);
    } 

    static std::unique_ptr<IBacteria> createOnClick(BacteriaType type, const glm::vec2& clickPosition){
        glm::vec2 offset = glm::circularRand(5.0f); 
        glm::vec2 spawnPos = clickPosition + offset;
        glm::vec4 position(spawnPos, 0.0f, 1.0f);
        return std::make_unique<Bacteria>(position, type);
    } 
};
