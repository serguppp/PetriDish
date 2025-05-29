#pragma once

#include "IBacteria.h" 
#include "BacteriaStats.h"
#include <vector>    
#include <utility>  
#include <cmath> 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
                { // Dwa połączone okręgi
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
                { // Nieregularny kształt
                    {0.0f, 1.5f}, {1.0f, 1.2f}, {1.5f, 0.5f}, {1.2f, -0.5f},
                    {0.5f, -1.5f}, {-0.5f, -1.2f}, {-1.5f, -0.5f}, {-1.0f, 1.0f}
                }
            };

        case BacteriaType::Bacillus:
            {
                // Elipsa
                std::vector<std::pair<float, float>> ellipseVertices;
                const int segments = 16; // Liczba segmentów do aproksymacji elipsy
                const float radiusX = 1.5f; // Promień w osi X
                const float radiusY = 0.6f; // Promień w osi Y 
                for (int i = 0; i <= segments; ++i) { 
                    float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * static_cast<float>(M_PI);
                    ellipseVertices.push_back({radiusX * std::cos(angle), radiusY * std::sin(angle)});
                }
                return {
                    1.2f, 
                    11.0f, 
                    0.25f, 
                    ellipseVertices
                };
            }

        default: // Domyślny kształt -kwadrat
            return {
                1.0f,
                15.0f,
                0.0f,
                {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}}
            };
    }
}