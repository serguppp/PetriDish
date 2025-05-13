#pragma once

#include <vector>

struct BacteriaStats {
    float health;
    float divisionInterval;
    float antibioticResistance;
    std::vector<std::pair<int, int>> circuit;
};