#pragma once

#include <functional>
#include "imgui.h"
#include "../Simulation/IBacteria.h" 

class GUIRenderer {
private:
    float antibioticStrength;
    float antibioticRadius;
    int addBacteriaCount; 
    BacteriaType selectedBacteriaType;
    float lightRange;

    ImVec2 currentMouseScreenPos; 
    bool isWaitingForBacteriaPlacement;
    bool isWaitingForAntibioticPlacement;
    size_t currentBacteriaCountDisplay;

public:
    GUIRenderer();

    std::function<void(BacteriaType type, int count, int screenX, int screenY)> onAddBacteria;
    std::function<void(float strength, float radius, int screenX, int screenY)> onApplyAntibiotic;
    std::function<void(float range)> onLightRangeChanged; 

    void render(const glm::vec2& viewOffset, float zoomLevel, int windowHeight, bool is3DView); 
    void setBacteriaCount(size_t count);

};
