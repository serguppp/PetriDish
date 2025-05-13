#pragma once

#include "IBacteria.h"
#include "BacteriaStats.h"
#include <unordered_map>

inline BacteriaStats getStatsForType(BacteriaType type) {
    //return { health, divisionInterval, antibioticResistance, circuit}
    switch (type) {
        case BacteriaType::Cocci:
            return {1.0f, 5.0f, 0.2f, {{-5, -5}, {5, -5}, {5, 5}, {-5, 5}}};
        case BacteriaType::Diplococcus:
            return {1.0f, 4.0f, 0.4f, {{-8, -4}, {8, -4}, {8, 4}, {-8, 4}}};
        case BacteriaType::Staphylococci:
            return {1.0f, 6.0f, 0.1f, {{-10, -10}, {10, -10}, {10, 10}, {-10, 10}}};
        default:
            return {1.0f, 5.0f, 0.0f, {{-5, -5}, {5, -5}, {5, 5}, {-5, 1}}}; 
    }
}
