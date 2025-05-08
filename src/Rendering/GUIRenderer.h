#pragma once

#include <functional>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../Simulation/IBacteria.h" 

class GUIRenderer {
private:
    float divisionInterval = 10.0f;
    float antibioticStrength = 0.2f;
    float antibioticRadius = 20.0f; 
    BacteriaType selectedBacteriaType = BacteriaType::Cocci; //Domyslnie Cocci

public:
    // Funkcje callback
    std::function<void(BacteriaType, int, int)> onAddBacteria; 
    std::function<void(int, int, float, float)> onApplyAntibiotic; 
    std::function<void(float)> onDivisionIntervalChanged;

    // Metoda renderująca GUI
    void render();

    // Metoda do ustawienia pozycji kliknięcia myszy 
    void setMouseClickPosition(int x, int y);

private:
    int mouseClickX = -1;
    int mouseClickY = -1;
};
