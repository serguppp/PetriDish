#pragma once

#include "IBacteria.h"
#include <algorithm>

class Coccus : public IBacteria {
public:
    Coccus(int x, int y)
        : x(x), y(y), health(0.5f), divisionTimer(0.0f), divisionInterval(10.0f) {
    }

    // Metoda update - wywoływana na każdej klatce
    void update(float deltaTime) override {
        if (health > 0.0f) {
            divisionTimer += deltaTime;
        }
    }

    // Sprawdzenie, czy bakteria może się podzielić
    bool canDivide() const override {
        return divisionTimer >= divisionInterval && isAlive();
    }

    // Metoda do aplikacji antybiotyku
    void applyAntibiotic(float intensity) override {
        // Słabe bakterie szybko tracą zdrowie pod wpływem antybiotyku
        float damage = intensity * 0.6f;  // Słabsze bakterie tracą więcej zdrowia
        health = std::max(0.0f, health - damage);
    }

    // Clone - tworzymy nową instancję tego samego typu
    IBacteria* clone() const override {
        return new Coccus(*this);
    }

    // Sprawdzamy, czy bakteria jest żywa
    bool isAlive() const {
        return health > 0.0f;
    }

    // Resetowanie timera podziału
    void resetDivisionTimer() {
        divisionTimer = 0.0f;
    }

    // Gettery do pozycji i zdrowia
    int getX() const { return x; }
    int getY() const { return y; }
    float getHealth() const { return health; }

private:
    float health;          // 0.0 (martwa) - 1.0 (zdrowa)
    float divisionTimer;   // Czas, który upłynął od ostatniego podziału
    float divisionInterval; // Czas między podziałami
    int x, y;              // Pozycja na szalce
};
