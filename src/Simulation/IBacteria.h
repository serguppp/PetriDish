#pragma once

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
    virtual void die() = 0;
    virtual void setPosition(int x, int y) = 0; 

    virtual int getX() const = 0;
    virtual int getY() const = 0;
    virtual float getHealth() const = 0;
    virtual BacteriaType getBacteriaType() const = 0;
    virtual std::vector<std::pair<int, int>>& getCircuit() = 0; 
    virtual void setCircuitForType() = 0; 
};
