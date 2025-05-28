#pragma once

#include "IBacteria.h"
#include "BacteriaStatsProvider.h" 

#include <glm/glm.hpp>      
#include "glm/gtc/random.hpp" 

#include <vector>
#include <utility> 
#include <iostream> 
#include <random> 
#include <chrono> 

class Bacteria : public IBacteria {
private:
    glm::vec4 position;
    BacteriaStats stats; 
    BacteriaType bacteriaType;
    float divisionTimer; 
    float radius;

public:
    Bacteria(glm::vec4 initialPosition, BacteriaType type);
    ~Bacteria() override = default;

    void update(float deltaTime) override;
    bool canDivide() const override;
    void applyAntibiotic(float intensity) override;
    IBacteria* clone() const override;
    void resetDivisionTimer() override;
    bool isAlive() const override;

    float getHealth() const override;
    glm::vec4 getPos() const override;
    BacteriaType getBacteriaType() const override;
    const std::vector<std::pair<float, float>>& getCircuit() const override;
    void setPos(const glm::vec4& newPosition) override;
    
};
