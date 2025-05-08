#include "GUIRenderer.h" 

void GUIRenderer::render() {
    // Rozpocznij nowe okno ImGui
    ImGui::Begin("Sterowanie Symulacja");

    // Sekcja dodawania bakterii
    ImGui::Text("Dodaj Bakterie:");
    const char* bacteriaTypes[] = { "Cocci", "Diplococcus", "Staphylococci" };
    static int currentBacteriaTypeIndex = 0;
    ImGui::Combo("Typ Bakterii", &currentBacteriaTypeIndex, bacteriaTypes, IM_ARRAYSIZE(bacteriaTypes));
    selectedBacteriaType = static_cast<BacteriaType>(currentBacteriaTypeIndex);

    // Przycisk dodający bakterię - wywołuje callback z wybranym typem i pozycją kliknięcia myszy
    // Pozycja myszy musi być ustawiona przed wywołaniem render()
    if (ImGui::Button("Dodaj Bakterie w miejscu klikniecia")) {
        if (onAddBacteria && mouseClickX != -1 && mouseClickY != -1) {
            onAddBacteria(selectedBacteriaType, mouseClickX, mouseClickY);
            // Resetuj pozycję kliknięcia po użyciu
            mouseClickX = -1;
            mouseClickY = -1;
        } else if (mouseClickX == -1 || mouseClickY == -1) {
            // Komunikat dla użytkownika, aby kliknął na szalkę
            ImGui::SameLine();
            ImGui::Text("Kliknij na szalke, aby dodac.");
        }
    }


    // Sekcja parametrów symulacji
    ImGui::Text("Parametry Symulacji:");

    // Suwak tempa podziału (zakres: 1s - 60s)
    if (ImGui::SliderFloat("Interwal Podzialu", &divisionInterval, 1.0f, 60.0f)) {
        if (onDivisionIntervalChanged) onDivisionIntervalChanged(divisionInterval);
    }

    // Sekcja antybiotyku
    ImGui::Text("Antybiotyk:");

    // Suwak intensywności antybiotyku
    ImGui::SliderFloat("Sila Antybiotyku", &antibioticStrength, 0.0f, 1.0f);

    // Suwak promienia antybiotyku
    ImGui::SliderFloat("Promien Antybiotyku", &antibioticRadius, 10.0f, 100.0f); // Przykładowy zakres

    // Przycisk aplikacji antybiotyku - wywołuje callback z pozycją kliknięcia myszy, promieniem i siłą
    if (ImGui::Button("Aplikuj Antybiotyk w miejscu klikniecia")) {
        if (onApplyAntibiotic && mouseClickX != -1 && mouseClickY != -1) {
            onApplyAntibiotic(mouseClickX, mouseClickY, antibioticRadius, antibioticStrength);
            // Resetuj pozycję kliknięcia po użyciu
            mouseClickX = -1;
            mouseClickY = -1;
        } else if (mouseClickX == -1 || mouseClickY == -1) {
             // Komunikat dla użytkownika, aby kliknął na szalkę
            ImGui::SameLine();
            ImGui::Text("Kliknij na szalke, aby zaaplikowac.");
        }
    }

    // Tutaj można dodać inne elementy GUI, np. sterowanie zoomem, pauza/wznowienie, statystyki

    ImGui::End(); // Zakończ okno ImGui
}

// Metoda do ustawienia pozycji kliknięcia myszy
void GUIRenderer::setMouseClickPosition(int x, int y) {
    mouseClickX = x;
    mouseClickY = y;
}
