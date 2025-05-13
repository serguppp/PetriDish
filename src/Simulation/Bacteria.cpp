#include "Bacteria.h"
#include <iostream> 

Bacteria::Bacteria(glm::vec4 initialPosition, BacteriaType type)
    : position(initialPosition),
      bacteriaType(type),
      stats(getStatsForType(type)), 
      divisionTimer(0.0f) {
}

void Bacteria::update(float deltaTime) {
    if (!isAlive()) {
        return;
    }
    divisionTimer += deltaTime;
}

bool Bacteria::canDivide() const {

    return isAlive() && stats.health > 0.7f && divisionTimer >= stats.divisionInterval; 
}

void Bacteria::applyAntibiotic(float intensity) {
    if (!isAlive()) return;

    float damage = intensity * (1.0f - stats.antibioticResistance);
    stats.health -= damage;

    if (stats.health < 0.0f) 
        stats.health = 0.0f;
    
}

IBacteria* Bacteria::clone() const {
    Bacteria* newBacteria = new Bacteria(this->position, this->bacteriaType);
    return newBacteria;
}

void Bacteria::resetDivisionTimer() {
    divisionTimer = 0.0f;
}

bool Bacteria::isAlive() const {
    return stats.health > 0.0f;
}

float Bacteria::getHealth() const {
    return stats.health;
}

glm::vec4 Bacteria::getPos() const {
    return position;
}

BacteriaType Bacteria::getBacteriaType() const {
    return bacteriaType;
}

// Return a const reference to the circuit
const std::vector<std::pair<float, float>>& Bacteria::getCircuit() const {
    return stats.circuit;
}

void Bacteria::setPos(const glm::vec4& newPosition) {
    position = newPosition;
}
