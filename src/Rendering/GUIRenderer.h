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

    ImVec2 currentMouseScreenPos; 

    bool isWaitingForBacteriaPlacement;
    bool isWaitingForAntibioticPlacement;

    size_t currentBacteriaCountDisplay; 

public:
    std::function<void(BacteriaType type, int count, int screenX, int screenY)> onAddBacteria;
    std::function<void(float strength, float radius, int screenX, int screenY)> onApplyAntibiotic;
    std::function<void(float interval)> onDivisionIntervalChanged; 

    GUIRenderer();
    void render(); 

    void setBacteriaCount(size_t count);

};
