#pragma once

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <utility>

enum class BacteriaType {
    Cocci,
    Diplococcus,
    Staphylococci
};

class IBacteria {
public:
    virtual ~IBacteria() = default; 

    virtual void update(float deltaTime) = 0;
    virtual bool canDivide() const = 0;
    virtual void applyAntibiotic(float intensity) = 0;
    virtual IBacteria* clone() const = 0; 
    virtual void resetDivisionTimer() = 0;
    virtual bool isAlive() const = 0;

    virtual float getHealth() const = 0;
    virtual glm::vec4 getPos() const = 0;
    virtual BacteriaType getBacteriaType() const = 0;
    virtual std::vector<std::pair<int, int>>& getCircuit() = 0; 
};
