#pragma once

#include "IBacteria.h" 
#include "BacteriaStats.h"
#include <vector>    
#include <utility>  

inline BacteriaStats getStatsForType(BacteriaType type) {
    // format: {health, divisionInterval, antibioticResistance, circuit_vertices}
    switch (type) {
        case BacteriaType::Cocci:
            return {
                1.0f,  // health
                8.0f,  // divisionInterval
                0.2f,  // antibioticResistance
                {      // circuit
                    {0.0f, 1.0f}, {0.707f, 0.707f}, {1.0f, 0.0f}, {0.707f, -0.707f},
                    {0.0f, -1.0f}, {-0.707f, -0.707f}, {-1.0f, 0.0f}, {-0.707f, 0.707f}
                }
            };
        case BacteriaType::Diplococcus:
            return {
                1.0f,
                10.0f,
                0.3f,
                {
                    // Czesc pierwsza
                    {-0.7f, 1.0f}, {-0.0f, 0.707f}, {0.2f, 0.0f}, {-0.0f, -0.707f},
                    {-0.7f, -1.0f}, {-1.4f, -0.707f}, {-1.6f, 0.0f}, {-1.4f, 0.707f},
                     // Czesc druga
                    {0.7f, 1.0f}, {1.4f, 0.707f}, {1.6f, 0.0f}, {1.4f, -0.707f},
                    {0.7f, -1.0f}, {0.0f, -0.707f}, {-0.2f, 0.0f}, {0.0f, 0.707f}
                }
            };
        case BacteriaType::Staphylococci:
            return {
                0.8f,
                12.0f,
                0.1f,
                {
                    {0.0f, 1.5f}, {1.0f, 1.2f}, {1.5f, 0.5f}, {1.2f, -0.5f},
                    {0.5f, -1.5f}, {-0.5f, -1.2f}, {-1.5f, -0.5f}, {-1.0f, 1.0f}
                }
            };

        case BacteriaType::Bacillus:
            return {
                1.2f,  
                11.0f, 
                0.25f, 
                {     
                    {-1.5f, 0.4f}, {1.5f, 0.4f}, {1.5f, -0.4f}, {-1.5f, -0.4f}
                }
            };

        default:
            return {
                1.0f,
                15.0f,
                0.0f,
                {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}}
            };
    }
}