#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include "IBacteria.h"
#include "Bacteria.h"
#include <memory>

class BacteriaFactory {
public:
    static std::unique_ptr<IBacteria> createAtXY(BacteriaType type, int x, int y){
        glm::vec4 position(x, y, 0.0f, 1.0f);
        return std::make_unique<Bacteria>(position, type);
    } 

    static std::unique_ptr<IBacteria> createAtPosition(BacteriaType type, const glm::vec4& position) {
        return std::make_unique<Bacteria>(position, type);
    }
};
