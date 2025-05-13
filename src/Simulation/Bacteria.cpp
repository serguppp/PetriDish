#include "Bacteria.h"

Bacteria::Bacteria(glm::vec4 position, BacteriaType bacteriaType)
    : position(position), bacteriaType(bacteriaType), stats(getStatsForType(bacteriaType)), divisionTimer(0.0f) {}

void Bacteria::update(float deltaTime) {
    divisionTimer += deltaTime;
}

bool Bacteria::canDivide() const {
    return stats.health > 0.5f && divisionTimer >= stats.divisionInterval;
}

void Bacteria::applyAntibiotic(float intensity) {
    float damage = intensity * (1.0f - stats.antibioticResistance);
    stats.health -= damage;
    if (stats.health < 0.0f) stats.health = 0.0f;
}

IBacteria* Bacteria::clone() const {
    return new Bacteria(position, bacteriaType);
}

void Bacteria::resetDivisionTimer() {
    divisionTimer = 0.0f;
}

