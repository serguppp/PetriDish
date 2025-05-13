#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Rendering/Renderer.h"
#include "Rendering/GUIRenderer.h"
#include "Simulation/Bacteria.h" 
#include "Simulation/BacteriaFactory.h"

#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <functional> 

void setupImGUI(GLFWwindow* window){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void cleanupGUI(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void setupCallbacks(GUIRenderer& guiRenderer, std::vector<std::unique_ptr<IBacteria>>& allBacteria ){
    guiRenderer.onAddBacteria = [&](BacteriaType type, int x, int y) {
        glm::vec2 clickPosition(x, y);
        allBacteria.push_back(BacteriaFactory::createOnClick(type, clickPosition));
    };   
    
    guiRenderer.onApplyAntibiotic = [&](float antibioticStrength, float antibioticRadius, int x, int y) {
    
    };    

    guiRenderer.onDivisionIntervalChanged = [&](float divisionInterval) {

    };    
}
int main() {
    Renderer renderer;
    GUIRenderer guiRenderer;
    std::vector<std::unique_ptr<IBacteria>> allBacteria;
    
    setupCallbacks(guiRenderer, allBacteria);
    setupImGUI(renderer.getWindow());

    // Główna pętla programu
    while (!glfwWindowShouldClose(renderer.getWindow())) {
        // Przetwarzanie zdarzeń (np. wejście z klawiatury)
        glfwPollEvents();

        // Rozpoczęcie klatki ImGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render elementów UI
        guiRenderer.render();

        //Renderowanie klatek 
        renderer.beginFrame();

        renderer.renderColony(allBacteria);
        
        // Renderowanie klatki ImGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        renderer.endFrame();
    }

    cleanupGUI();
    glfwTerminate();
    return 0;
}
