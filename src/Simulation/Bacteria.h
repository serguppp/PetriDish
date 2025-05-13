#pragma once

#include "IBacteria.h" 
#include "BacteriaStatsProvider.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
#include <utility>

class Bacteria : public IBacteria {
private:
    float divisionTimer;   
    glm::vec4 position;
    BacteriaStats stats;
    BacteriaType bacteriaType;
public:
    Bacteria(glm::vec4 position, BacteriaType bacteriaType);
    ~Bacteria() override = default;

    void update(float deltaTime) override;
    bool canDivide() const override;
    void applyAntibiotic(float intensity) override;
    IBacteria* clone() const override;
    void resetDivisionTimer() override;
    
    bool isAlive() const override { return stats.health > 0.0f;}
    glm::vec4 getPos() const override { return position; }
    float getHealth() const override { return stats.health; }
    BacteriaType getBacteriaType() const override { return bacteriaType; }
    std::vector<std::pair<int, int>>& getCircuit() override { return stats.circuit; }

};
