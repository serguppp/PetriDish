#pragma once

#include "IBacteria.h" 
#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
#include <utility>

class Coccus : public IBacteria {
private:
    float health;          // 0.0 (martwa) - 1.0 (zdrowa)
    float divisionTimer;   // Czas, który upłynął od ostatniego podziału
    float divisionInterval; // Czas między podziałami
    int x, y;              // Pozycja na szalce (indeksy siatki)
    std::vector<std::pair<int, int>> circuit; // Pozycje punktów wyznaczających obwód kształtu bakterii
    BacteriaType bacteriaType;
    static std::default_random_engine engine; 

public:
    Coccus(int x, int y, BacteriaType bacteriaType);

    void update(float deltaTime) override;
    bool canDivide() const override;
    void applyAntibiotic(float intensity) override;
    IBacteria* clone() const override;
    void resetDivisionTimer() override;
    bool isAlive() const override;
    void die() override;
    void setPosition(int x, int y) override; 
    void setCircuitForType() override;

    int getX() const override { return x; }
    int getY() const override { return y; }
    float getHealth() const override { return health; }
    BacteriaType getBacteriaType() const override { return this->bacteriaType; }
    std::vector<std::pair<int, int>>& getCircuit() override { return circuit; }

    ~Coccus();
};
