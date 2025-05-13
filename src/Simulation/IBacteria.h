#pragma once

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <utility> 
#include <string>  

enum class BacteriaType {
    Cocci,
    Diplococcus,
    Staphylococci
};

struct BacteriaStats;

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

    virtual const std::vector<std::pair<float, float>>& getCircuit() const = 0; 

    virtual void setPos(const glm::vec4& newPosition) = 0;
};
