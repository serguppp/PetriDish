#include "GUIRenderer.h"
#include "imgui_impl_glfw.h"    
#include "imgui_impl_opengl3.h" 

// Constructor
GUIRenderer::GUIRenderer()
    : antibioticStrength(0.5f),       
      antibioticRadius(50.0f),        
      addBacteriaCount(100),          
      selectedBacteriaType(BacteriaType::Cocci),
      currentMouseScreenPos(0,0),
      isWaitingForBacteriaPlacement(false),
      isWaitingForAntibioticPlacement(false),
      currentBacteriaCountDisplay(0) {}

void GUIRenderer::setBacteriaCount(size_t count) {
    currentBacteriaCountDisplay = count;
}

void GUIRenderer::render() {
    ImGui::Begin("Symulacja");

    // --- Licznik FPS ---
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();

    // --- Pozycja myszki ---
    currentMouseScreenPos = ImGui::GetMousePos();
    ImGui::Text("Pozycja myszki: X: %.1f, Y: %.1f", currentMouseScreenPos.x, currentMouseScreenPos.y);
    ImGui::Separator();

    // --- Liczba bakterii ---
    ImGui::Text("Liczba bakterii: %zu", currentBacteriaCountDisplay);
    ImGui::Separator();

    // --- Sekcja dodawania bakterii---
    ImGui::Text("Dodaj bakterie:");
    const char* bacteriaTypeNames[] = {"Cocci", "Diplococcus", "Staphylococci"};
    static int currentBacteriaTypeIndex = static_cast<int>(selectedBacteriaType);
    if (ImGui::Combo("Typ", &currentBacteriaTypeIndex, bacteriaTypeNames, IM_ARRAYSIZE(bacteriaTypeNames))) {
        selectedBacteriaType = static_cast<BacteriaType>(currentBacteriaTypeIndex);
    }
    ImGui::SliderInt("Liczba", &addBacteriaCount, 1, 500);

    if (isWaitingForBacteriaPlacement) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Kliknij na ekran, aby dodac bakterię %s.", bacteriaTypeNames[currentBacteriaTypeIndex]);
        if (ImGui::Button("Anuluj dodawanie")) {
            isWaitingForBacteriaPlacement = false;
        }
    } else {
        if (ImGui::Button("Dodaj bakterie")) {
            isWaitingForBacteriaPlacement = true;
            isWaitingForAntibioticPlacement = false; 
        }
    }

    if (isWaitingForBacteriaPlacement && ImGui::IsMouseClicked(0) && !ImGui::GetIO().WantCaptureMouse) {
        if (onAddBacteria) {
            onAddBacteria(selectedBacteriaType, addBacteriaCount, static_cast<int>(currentMouseScreenPos.x), static_cast<int>(currentMouseScreenPos.y));
        }
        isWaitingForBacteriaPlacement = false;
    }
    ImGui::Separator();


    // --- Sekcja dodawania antybiotyku ---
    ImGui::Text("Aplikuj antybiotyk:");
    ImGui::SliderFloat("Sila", &antibioticStrength, 0.05f, 1.0f);
    ImGui::SliderFloat("Zasieg dzialania", &antibioticRadius, 10.0f, 200.0f);

    if (isWaitingForAntibioticPlacement) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Kliknij na ekran, aby aplikowac antybiotyk.");
        if (ImGui::Button("Anuluj aplikowanie")) {
            isWaitingForAntibioticPlacement = false;
        }
    } else {
        if (ImGui::Button("Aplikuj antybiotyk")) {
            isWaitingForAntibioticPlacement = true;
            isWaitingForBacteriaPlacement = false; 
        }
    }

    if (isWaitingForAntibioticPlacement && ImGui::IsMouseClicked(0) && !ImGui::GetIO().WantCaptureMouse) {
        if (onApplyAntibiotic) {
            onApplyAntibiotic(antibioticStrength, antibioticRadius, static_cast<int>(currentMouseScreenPos.x), static_cast<int>(currentMouseScreenPos.y));
        }
        isWaitingForAntibioticPlacement = false; 
    }
    ImGui::Separator();

    ImGui::End();
}
