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
#include "Rendering/Camera.h" 
#include "Simulation/Bacteria.h"
#include "Simulation/BacteriaFactory.h"

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const size_t MAX_BACTERIA_COUNT = 10000;

Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        camera.toggleViewMode();
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camera.reset3DView();
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    camera.handleScroll(window, xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); 
    if (ImGui::GetIO().WantCaptureMouse) {
        camera.isPanning2D = false; 
        camera.isRotating3D = false;
        return;
    }
    camera.handleMouseButton(window, button, action);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    if (ImGui::GetIO().WantCaptureMouse) return;
    camera.handleMouseMove(window, xpos, ypos);
}

void setupImGUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void cleanupGUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

    // === sekcja UPDATE ===
    // co klatkę aktualizujemy divisionTimer bakterii oraz sprawdzamy, czy nadaje się do usunięcia
    // a także co klatkę aktualizujemy działanie antybiotyku na bakterie
void update(std::vector<std::unique_ptr<IBacteria>>& allBacteria, float deltaTime) {
    for (auto& bacteria : allBacteria) {
        if (bacteria) {
            bacteria->update(deltaTime);
        }
    }

    std::vector<std::unique_ptr<IBacteria>> newBacteria;
    for (auto& bacteria : allBacteria) {
        if (bacteria && bacteria->canDivide()) {
            if (glm::linearRand(0.0f, 1.0f) < 0.05f) { // 5% szans na podział
                IBacteria* child = bacteria->clone();
                if (child) {
                    newBacteria.push_back(std::unique_ptr<IBacteria>(child));
                }
            }
            bacteria->resetDivisionTimer();
        }
    }
    allBacteria.insert(allBacteria.end(), std::make_move_iterator(newBacteria.begin()), std::make_move_iterator(newBacteria.end()));

    allBacteria.erase(
        std::remove_if(allBacteria.begin(), allBacteria.end(),
                       [](const std::unique_ptr<IBacteria>& b) { return !b || !b->isAlive(); }),
        allBacteria.end()
    );
}


void setupGuiCallbacks(GUIRenderer& guiRenderer, Renderer& renderer, std::vector<std::unique_ptr<IBacteria>>& allBacteria) {
    guiRenderer.onAddBacteria = [&](BacteriaType type, int bacteriaCount, int x_screen_raw, int y_screen_raw) {
        glm::vec2 screen_pos_gl(static_cast<float>(x_screen_raw), static_cast<float>(WINDOW_HEIGHT - y_screen_raw));
        glm::vec2 world_click_center_pos = camera.screenToWorld2D(screen_pos_gl);

        glm::vec3 clickCenter(world_click_center_pos.x, world_click_center_pos.y, 0.1f);
        float radius = 2.0f;

        for (int i = 0; i < bacteriaCount; ++i) {
            glm::vec2 randomOffset = glm::gaussRand(glm::vec2(0.0f), glm::vec2(radius));
            float offsetZ = 1.75f + static_cast<float>(i) * glm::linearRand(0.0f, 0.001f);
            glm::vec3 spawnPosition = clickCenter + glm::vec3(randomOffset.x, randomOffset.y, offsetZ);
            allBacteria.push_back(BacteriaFactory::createAtPosition(type, glm::vec4(spawnPosition, 1.0f)));
        }
    };

    guiRenderer.onApplyAntibiotic = [&](float antibioticStrength, float antibioticRadius, int x_screen_raw, int y_screen_raw) {
        glm::vec2 screen_pos_gl(static_cast<float>(x_screen_raw), static_cast<float>(WINDOW_HEIGHT - y_screen_raw));
        glm::vec2 world_click_center_pos = camera.screenToWorld2D(screen_pos_gl);
        renderer.addAntibioticEffect(world_click_center_pos, antibioticStrength, antibioticRadius);
        for (auto& bacteria : allBacteria) {
            if (bacteria && bacteria->isAlive()) {
                glm::vec4 bacteriaPosVec4 = bacteria->getPos();
                glm::vec2 bacteriaPos(bacteriaPosVec4.x, bacteriaPosVec4.y);
                float distance = glm::distance(world_click_center_pos, bacteriaPos);
                if (distance <= antibioticRadius) {
                    float strengthAtDistance = antibioticStrength * (1.0f - glm::smoothstep(0.0f, antibioticRadius, distance));
                    bacteria->applyAntibiotic(strengthAtDistance);
                }
            }
        }
    };

    guiRenderer.onLightRangeChanged = [&](float range) {
        renderer.setLightRange(range * 2); 
        renderer.setGlowRadius(10.0f * range / 100.0f); 
    };
}


int main() {
    // Inicjalizacja GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Utworzenie instancji Renderer która tworzy okno i inicjalizuje GLEW
    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!renderer.isInitialized()) {
        std::cerr << "Renderer initialization failed. Exiting." << std::endl;
        glfwTerminate();
        return -1;
    }
    GLFWwindow* window = renderer.getWindow();

    GUIRenderer guiRenderer;
    std::vector<std::unique_ptr<IBacteria>> allBacteria;

    // Ustawienie callbacków GLFW
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Inicjalizacja ImGui
    setupImGUI(window);
    // Ustawienie callbacków dla GUI 
    setupGuiCallbacks(guiRenderer, renderer, allBacteria);

    float lastFrameTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        deltaTime = glm::min(deltaTime, 0.1f); 

        glfwPollEvents();
        update(allBacteria, deltaTime);
        renderer.updateAntibioticEffects(deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        guiRenderer.setBacteriaCount(allBacteria.size());
        guiRenderer.render(camera.viewOffset, camera.currentZoomLevel, WINDOW_HEIGHT, camera.is3DView);

        renderer.beginFrame();

        // Poniższa macierz będzie przekazana do renderera i do shaderów
        glm::mat4 projectionMatrix = camera.getProjectionMatrix();
        glm::mat4 viewMatrix = camera.getViewMatrix();
        glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

        renderer.renderPetriDish(viewProjectionMatrix, viewMatrix);
        renderer.renderColony(allBacteria, camera.currentZoomLevel, viewProjectionMatrix); 
        renderer.renderAntibioticEffects(viewProjectionMatrix);
        renderer.renderGlowEffect(projectionMatrix, viewMatrix, camera.currentZoomLevel);

        // Renderowanie klatki ImGui na wierzchu sceny
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        renderer.endFrame();
    }

    cleanupGUI();
    glfwTerminate();
    return 0;
}