#include "Coccus.h"

std::default_random_engine Coccus::engine(std::random_device{}());

Coccus::Coccus(int x, int y, BacteriaType bacteriaType)
    : x(x), y(y), bacteriaType(bacteriaType), health(1.0f), divisionTimer(0.0f), divisionInterval(10.0f) {
    setCircuitForType();
}

// Metoda update wywoływana w każdej klatce
void Coccus::update(float deltaTime) {
    if (health > 0.0f) {
        divisionTimer += deltaTime;
    }
}

bool Coccus::canDivide() const {
    // Bakteria może się podzielić, jeśli jest żywa i minął odpowiedni czas
    return health > 0.0f && divisionTimer >= divisionInterval;
}

// Metoda do aplikacji antybiotyku
void Coccus::applyAntibiotic(float intensity) {
    if (health > 0.0f) {
        float damage = intensity * 0.1f;
        health = std::max(0.0f, health - damage);
    }
}

IBacteria* Coccus::clone() const {
    return new Coccus(this->x, this->y, this->bacteriaType);
}

// Resetowanie timera podziału
void Coccus::resetDivisionTimer() {
    divisionTimer = 0.0f;
}

bool Coccus::isAlive() const {
    return health > 0.0f;
}

void Coccus::die() {
    health = 0.0f;
}

void Coccus::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

void Coccus::setCircuitForType() {
    switch (bacteriaType) {
        case BacteriaType::Cocci:
            circuit = {{-5, -5}, {5, -5}, {5, 5}, {-5, 5}};
            break;
        case BacteriaType::Diplococcus:
            circuit = {{-8, -4}, {8, -4}, {8, 4}, {-8, 4}};
            break;
        case BacteriaType::Staphylococci:
            circuit = {{-10, -10}, {10, -10}, {10, 10}, {-10, 10}};
            break;
        default:
            circuit = {{-5, -5}, {5, -5}, {5, 5}, {-5, 1}}; 
    }
}

Coccus::~Coccus() {
}
