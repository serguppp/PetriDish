#include "Bacteria.h"

Bacteria::Bacteria(glm::vec4 initialPosition, BacteriaType type)
    : position(initialPosition),
      bacteriaType(type),
      stats(getStatsForType(type)), 
      divisionTimer(stats.divisionInterval) {
}

void Bacteria::update(float deltaTime) {
    if (!isAlive()) {
        return;
    }
    divisionTimer -= deltaTime;
}

bool Bacteria::canDivide() const {

    return isAlive() && stats.health > 0.7f && divisionTimer <= 0.0f; 
}

void Bacteria::applyAntibiotic(float intensity) {
    if (!isAlive()) return;

    float effectiveResistance = glm::clamp(stats.antibioticResistance, 0.0f, 0.95f); 
    float damage = intensity * (1.0f - effectiveResistance);
    
    if (damage > 0.0f) {
        stats.health -= damage;
        if (stats.health < 0.0f) {
            stats.health = 0.0f;
        }
    }
}

IBacteria* Bacteria::clone() const {
    float offsetRadius = 0.5f; 
    float offsetZ = 0.05f;
    glm::vec2 randomOffset = glm::diskRand(offsetRadius);
    glm::vec4 newPosition = position + glm::vec4(randomOffset.x, randomOffset.y, offsetZ, 0.0f);    

    const float maxZ = 2.0f;
    if (newPosition.z > maxZ) {
        newPosition.z = maxZ - glm::linearRand(0.0f, 0.1f); 
    }

    Bacteria* child = new Bacteria(newPosition, this->bacteriaType);
    return child;
}

void Bacteria::resetDivisionTimer() {
    divisionTimer = stats.divisionInterval;
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

const std::vector<std::pair<float, float>>& Bacteria::getCircuit() const {
    return stats.circuit;
}

void Bacteria::setPos(const glm::vec4& newPosition) {
    position = newPosition;
}
