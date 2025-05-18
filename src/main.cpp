#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"        

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

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const float MAX_ZOOM_LEVEL = 30.0f;
const float MIN_ZOOM_LEVEL = 0.2f;
const float ZOOM_SENSITIVITY = 0.1f;

float currentZoomLevel = 1.0f;
glm::vec2 viewOffset(0.0f, 0.0f); 
glm::vec2 lastMousePos(0.0f, 0.0f);
bool isPanning = false;

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
    if (ImGui::GetIO().WantCaptureMouse){ 
        return;
    }

    // Pobieranie pozycji kursora na ekranie
    double mouseX_screen, mouseY_screen;
    glfwGetCursorPos(window, &mouseX_screen, &mouseY_screen);

    // Konwertuj współrzędne Y myszy (od góry okna) na współrzędne OpenGL (od dołu okna)
    float screen_y_opengl = static_cast<float>(WINDOW_HEIGHT - mouseY_screen);
    glm::vec2 screen_mouse_pos(static_cast<float>(mouseX_screen), screen_y_opengl);

    // Obliczanie pozycji w świecie
    glm::vec2 world_pos_under_cursor_before_zoom = viewOffset + (screen_mouse_pos / currentZoomLevel);

    // Zoomowanie
    float oldZoomLevel = currentZoomLevel; 
    if (yoffset > 0){ 
        currentZoomLevel *= (1.0f + ZOOM_SENSITIVITY * 2.0f);
    }
    else if (yoffset < 0){ 
        currentZoomLevel /= (1.0f + ZOOM_SENSITIVITY * 2.0f);
    }
    currentZoomLevel = glm::clamp(currentZoomLevel, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);

    if (oldZoomLevel == currentZoomLevel) {
        return;
    }

    // Aktualizacja widoku po zoomie
    viewOffset = world_pos_under_cursor_before_zoom - (screen_mouse_pos / currentZoomLevel);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) { 
        isPanning = false;
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        isPanning = true;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        lastMousePos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
        isPanning = false;
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (isPanning) {
        glm::vec2 currentMousePos_screen(static_cast<float>(xpos), static_cast<float>(ypos));
        glm::vec2 delta_screen = currentMousePos_screen - lastMousePos;
        lastMousePos = currentMousePos_screen;

        viewOffset.x -= delta_screen.x / currentZoomLevel;
        viewOffset.y += delta_screen.y / currentZoomLevel;
    }
}

void setupImGUI(GLFWwindow *window){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, false); 
    ImGui_ImplOpenGL3_Init("#version 130");
}

void cleanupGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void setupGuiCallbacks(GUIRenderer &guiRenderer, Renderer& renderer, std::vector<std::unique_ptr<IBacteria>> &allBacteria){
    guiRenderer.onAddBacteria = [&](BacteriaType type, int bacteriaCount, int x_screen_raw, int y_screen_raw){
        float gl_screen_y = static_cast<float>(WINDOW_HEIGHT - y_screen_raw);
        glm::vec2 screen_pos_for_unproject(static_cast<float>(x_screen_raw), gl_screen_y);
        glm::vec2 world_click_center_pos = viewOffset + (screen_pos_for_unproject / currentZoomLevel);

        glm::vec3 clickCenter(world_click_center_pos.x, world_click_center_pos.y, 0.0f);
        // Promień rozrzutu 
        float offsetRadiusWorld = 2.0f; 

        for (int i = 0; i < bacteriaCount; ++i){
            glm::vec2 randomOffset = glm::gaussRand(glm::vec2(0.0f), glm::vec2(offsetRadiusWorld));
            glm::vec3 spawnPosition = clickCenter + glm::vec3(randomOffset.x, randomOffset.y, 0.0f);
            glm::vec4 finalPosition(spawnPosition, 1.0f);
            allBacteria.push_back(BacteriaFactory::createAtPosition(type, finalPosition));
        }
    };

    guiRenderer.onApplyAntibiotic = [&](float antibioticStrength, float antibioticRadius, int x_screen_raw, int y_screen_raw){
        float gl_screen_y = static_cast<float>(WINDOW_HEIGHT - y_screen_raw);
        glm::vec2 screen_pos_for_unproject(static_cast<float>(x_screen_raw), gl_screen_y);
        glm::vec2 world_click_center_pos = viewOffset + (screen_pos_for_unproject / currentZoomLevel);

        renderer.addAntibioticEffect(world_click_center_pos, antibioticStrength, antibioticRadius);

        for (auto& bacteria : allBacteria) {
            if (bacteria && bacteria->isAlive()) {
                glm::vec4 bacteriaPosVec4 = bacteria->getPos();
                glm::vec2 bacteriaPos(bacteriaPosVec4.x, bacteriaPosVec4.y);
                float distance = glm::distance(world_click_center_pos, bacteriaPos);
                if (distance <= antibioticRadius) {
                    float strengthAtDistance = antibioticStrength * (1.0f - (distance / antibioticRadius));
                    bacteria->applyAntibiotic(strengthAtDistance);
                }
            }
        }
    };

    guiRenderer.onLightRangeChanged = [&](float range) {
        renderer.setLightRange(range*1.4);
        renderer.setGlowRadius(range);
    };
}

int main(){
    // Inicjalizacja GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Utworzenie instancji Renderer która tworzy okno i inicjalizuje GLEW
    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!renderer.isInitialized()){ 
        std::cerr << "Renderer initialization failed. Exiting." << std::endl;
        glfwTerminate(); 
        return -1;
    }

    GUIRenderer guiRenderer;
    std::vector<std::unique_ptr<IBacteria>> allBacteria;

    // Ustawienie callbacków GLFW
    glfwSetScrollCallback(renderer.getWindow(), scroll_callback);
    glfwSetMouseButtonCallback(renderer.getWindow(), mouse_button_callback);
    glfwSetCursorPosCallback(renderer.getWindow(), cursor_position_callback);

    // Inicjalizacja ImGui
    setupImGUI(renderer.getWindow()); 

    // Ustawienie callbacków dla GUI 
    setupGuiCallbacks(guiRenderer, renderer, allBacteria);

    float lastFrameTime = 0.0f;

    // Główna pętla programu
    while (!glfwWindowShouldClose(renderer.getWindow())) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        glfwPollEvents();

        // --- sekcja UPDATE ---
        for (auto& bacteria : allBacteria) {
            if (bacteria) {
                bacteria->update(deltaTime);
            }
        }
        
        allBacteria.erase(
            std::remove_if(allBacteria.begin(), allBacteria.end(),
                           [](const std::unique_ptr<IBacteria>& b) { return !b || !b->isAlive(); }),
            allBacteria.end()
        );
        
        renderer.updateAntibioticEffects(deltaTime);        
        // -----------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        guiRenderer.setBacteriaCount(allBacteria.size());
        guiRenderer.render(viewOffset, currentZoomLevel, WINDOW_HEIGHT);  // Logika UI w ImGui

        // Rozpoczęcie klatki renderowania sceny
        renderer.beginFrame();

        // Ustawienie macierzy projekcji dla efektu zoomu i przesuwania
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // Obliczanie szerokości i wysokości widoku w jednostkach świata
        float view_width_world = static_cast<float>(WINDOW_WIDTH) / currentZoomLevel;
        float view_height_world = static_cast<float>(WINDOW_HEIGHT) / currentZoomLevel;

        // Definiowanie rogów szalki
        float ortho_left = viewOffset.x;
        float ortho_right = viewOffset.x + view_width_world;
        float ortho_bottom = viewOffset.y;
        float ortho_top = viewOffset.y + view_height_world;

        glOrtho(ortho_left, ortho_right, ortho_bottom, ortho_top, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW); 
        glLoadIdentity();           

        GLfloat projMatrixGL[16];
        GLfloat modelViewMatrixGL[16];
        glGetFloatv(GL_PROJECTION_MATRIX, projMatrixGL);
        glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrixGL);
        glm::mat4 projectionMatrix = glm::make_mat4(projMatrixGL);
        glm::mat4 modelViewMatrix = glm::make_mat4(modelViewMatrixGL);

        // Renderowanie kolonii bakterii
        renderer.renderColony(allBacteria, currentZoomLevel, viewOffset); 

        //Renderowanie antybiotyków
        renderer.renderAntibioticEffects(currentZoomLevel);

         // Renderowanie poświaty 
        renderer.renderGlowEffect(projectionMatrix, modelViewMatrix, currentZoomLevel);

        // Renderowanie klatki ImGui na wierzchu sceny
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Zakończenie klatki renderowania
        renderer.endFrame(); 
    }

    cleanupGUI();
    glfwTerminate();
    return 0;
}