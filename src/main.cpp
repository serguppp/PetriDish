#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Rendering/Renderer.h"
#include "Rendering/GUIRenderer.h"
#include "Simulation/Coccus.h" 

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

int main() {

    //*****************************************************/
    auto previousTime = std::chrono::steady_clock::now();
    int frameCount = 0;
    float elapsedTime = 0.0f;

    // Tworzymy obiekty
    Renderer renderer;
    GUIRenderer guiRenderer;
    setupImGUI(renderer.getWindow());

    //*****************************************************/

    //***********************************************************************

    // Główna pętla programu
    while (!glfwWindowShouldClose(renderer.getWindow())) {
        //**** Mechanizm FPS ****/
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> deltaTimeDuration = currentTime - previousTime;
        previousTime = currentTime;

        float dt = deltaTimeDuration.count();
        elapsedTime += dt;
        frameCount++;

        if (elapsedTime >= 1.0f) {
            std::cout << "FPS: " << frameCount << std::endl; 
            frameCount = 0;
            elapsedTime = 0.0f;
        }
        //***********************/

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

        // Renderowanie klatki ImGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        renderer.endFrame();

        //***********************************************************************
    }

    cleanupGUI();
    glfwTerminate();
    return 0;
}
