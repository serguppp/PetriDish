#pragma once

#include <functional>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../Simulation/IBacteria.h" 

class GUIRenderer {
private:
    float divisionInterval;
    float antibioticStrength;
    float antibioticRadius; 
    BacteriaType selectedBacteriaType; 

    int mouseClickX;
    int mouseClickY;
    ImVec2 mousePos;

    bool isWaitingForBacteriaPlacement;
    bool isWaitingForAntibioticPlacement;

public:
    // Funkcje callback
    std::function<void(BacteriaType, int, int)> onAddBacteria; 
    std::function<void(float, float, int, int)> onApplyAntibiotic; 
    std::function<void(float)> onDivisionIntervalChanged;
    
    GUIRenderer();
    // Metoda renderująca GUI
    void render();

    // Metoda do ustawienia pozycji kliknięcia myszy 
    void setMouseClickPosition(int x, int y);
};
