#include "GUIRenderer.h" 
#include <iostream>


GUIRenderer::GUIRenderer() : divisionInterval(10.0f), antibioticStrength(0.2f), antibioticRadius(20.0f),
    selectedBacteriaType(BacteriaType::Cocci), mouseClickX(-1), mouseClickY(-1),
    isWaitingForBacteriaPlacement(false), isWaitingForAntibioticPlacement(false) {}

void GUIRenderer::render() {
    ImGui::Begin("Sterowanie Symulacja");

    // Sekcja FPS
    static float lastTime = 0.0f;
    static int frameCount = 0;
    static float fps = 0.0f;

    float currentTime = (float)ImGui::GetTime();
    frameCount++;

    if (currentTime - lastTime >= 1.0f){
        fps = (float)frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }
    ImGui::Text("FPS: %.1f", fps);

    // Sekcja koordynatów
    ImGui::Text("Pozycja myszki:");
    mousePos = ImGui::GetMousePos();
    ImGui::Text("X: %.1f, Y: %.1f", mousePos.x, mousePos.y);

    // Sekcja dodawania bakterii
    ImGui::Text("Dodaj Bakterie:");
    const char* bacteriaTypes[] = { "Cocci", "Diplococcus", "Staphylococci" };

    static int currentBacteriaTypeIndex = 0;
    ImGui::Combo("Typ Bakterii", &currentBacteriaTypeIndex, bacteriaTypes, IM_ARRAYSIZE(bacteriaTypes));
    selectedBacteriaType = static_cast<BacteriaType>(currentBacteriaTypeIndex);

    if (isWaitingForBacteriaPlacement){
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Kliknij na szalke, aby dodac %s.", bacteriaTypes[currentBacteriaTypeIndex]);
        if (ImGui::Button("Anuluj dodawanie")){
            isWaitingForBacteriaPlacement=false;
            setMouseClickPosition(-1,-1);
        }

        if (mouseClickX != -1 && mouseClickY != -1){
            if (onAddBacteria){
                onAddBacteria(selectedBacteriaType, mouseClickX, mouseClickY);
                isWaitingForBacteriaPlacement=false;
                setMouseClickPosition(-1,-1);
            }
        }
    }   
    else {
        if (ImGui::Button("Dodaj bakterie")){
            isWaitingForBacteriaPlacement = true;
            if (isWaitingForAntibioticPlacement) isWaitingForAntibioticPlacement = false;
            setMouseClickPosition(-1,-1);
        }
    }

    if (ImGui::IsMouseClicked(0)) {
        mouseClickX = (int)mousePos.x;
        mouseClickY = (int)mousePos.y;
    }

    ImGui::Text("Parametry Symulacji:");

    // Suwak tempa podziału (1-60s)
    if (ImGui::SliderFloat("Interwal Podzialu", &divisionInterval, 1.0f, 60.0f)) {
        if (onDivisionIntervalChanged) onDivisionIntervalChanged(divisionInterval);
    }

    // Sekcja antybiotyku
    ImGui::Text("Antybiotyk:");

    // Suwak intensywności antybiotyku
    ImGui::SliderFloat("Sila Antybiotyku", &antibioticStrength, 0.0f, 1.0f);

    // Suwak promienia antybiotyku
    ImGui::SliderFloat("Promien Antybiotyku", &antibioticRadius, 10.0f, 100.0f);

    // Przycisk aplikacji antybiotyku 
    if (isWaitingForAntibioticPlacement){
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Kliknij na szalke, aby aplikować antybiotyk");
        if (ImGui::Button("Anuluj dodawanie")){
            isWaitingForAntibioticPlacement=false;
            setMouseClickPosition(-1,-1);
        }

        if (mouseClickX != -1 && mouseClickY != -1){
            if (onApplyAntibiotic){
                onApplyAntibiotic(antibioticStrength, antibioticRadius, mouseClickX, mouseClickY);
                isWaitingForAntibioticPlacement=false;
                setMouseClickPosition(-1,-1);
            }
        }
    }   
    else {
        if (ImGui::Button("Aplikuj antybiotyk")){
            isWaitingForAntibioticPlacement = true;
            if (isWaitingForBacteriaPlacement) isWaitingForBacteriaPlacement = false;
            setMouseClickPosition(-1,-1);
        }
    }

    ImGui::End();
}

void GUIRenderer::setMouseClickPosition(int x, int y) {
    mouseClickX = x;
    mouseClickY = y;
}
