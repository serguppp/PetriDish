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


const size_t MAX_BACTERIA_COUNT = 10000; 
bool is3DView = false;

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// =================
// === Kamera 2D ===
const float MAX_ZOOM_LEVEL = 30.0f;
const float MIN_ZOOM_LEVEL = 0.2f;
const float ZOOM_SENSITIVITY = 0.1f;

float currentZoomLevel = 1.0f;
glm::vec2 viewOffset(0.0f, 0.0f); 
glm::vec2 lastMousePos(0.0f, 0.0f);
bool isPanning = false;

// =================

// =================
// === Kamera 3D ===
glm::vec3 cameraPos3D = glm::vec3(0.0f, -15.0f, 10.0f); // Pozycja kamery w 3D
glm::vec3 cameraTarget3D = glm::vec3(0.0f, 0.0f, 0.0f); // Na co kamera patrzy
glm::vec3 cameraUp3D = glm::vec3(0.0f, 0.0f, 1.0f);    // Orientacja kamery 

float cameraDistance = 20.0f; // Odległość kamery od celu
float cameraYaw = 0.0f;       // Kąt obrotu wokół osi Y
float cameraPitch = glm::radians(-30.0f); // Kąt obrotu góra/dół

bool isPanning3D = false;
glm::vec2 lastMousePos3D(0.0f, 0.0f);

const float ROTATION_SENSITIVITY = 0.005f;
const float ZOOM_3D_SENSITIVITY = 0.5f;
const float MIN_CAMERA_DISTANCE = 2.0f;
const float MAX_CAMERA_DISTANCE = 100.0f;
const float MIN_PITCH = glm::radians(-89.0f);
const float MAX_PITCH = glm::radians(0.0f); 

// =================

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        is3DView = !is3DView;
    }

    if (is3DView && key == GLFW_KEY_R && action == GLFW_PRESS) {
        cameraDistance = 20.0f;
        cameraYaw = 0.0f;
        cameraPitch = glm::radians(-30.0f);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
    if (ImGui::GetIO().WantCaptureMouse){ 
        return;
    }

     if (is3DView) {
        cameraDistance -= static_cast<float>(yoffset) * ZOOM_3D_SENSITIVITY;
        cameraDistance = glm::clamp(cameraDistance, MIN_CAMERA_DISTANCE, MAX_CAMERA_DISTANCE);
    }
    else{
        // Pobieranie pozycji kursora na ekranie
        double mouseX_screen, mouseY_screen;
        glfwGetCursorPos(window, &mouseX_screen, &mouseY_screen);

        // Konwert współrzędne Y myszy (od góry okna) na współrzędne OpenGL (od dołu okna)
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

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) { isPanning = false; return; }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (is3DView) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) isPanning3D = (action == GLFW_PRESS);
        if (isPanning3D) lastMousePos3D = glm::vec2(xpos, ypos);
    } else {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) isPanning = (action == GLFW_PRESS);
        if (isPanning) lastMousePos = glm::vec2(xpos, ypos);
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (is3DView && isPanning3D) {
        glm::vec2 delta = glm::vec2(xpos, ypos) - lastMousePos3D;
        lastMousePos3D = glm::vec2(xpos, ypos);
        cameraYaw -= delta.x * ROTATION_SENSITIVITY;
        cameraPitch = glm::clamp(cameraPitch - delta.y * ROTATION_SENSITIVITY, MIN_PITCH, MAX_PITCH);
    } else if (isPanning) {
        glm::vec2 delta = glm::vec2(xpos, ypos) - lastMousePos;
        lastMousePos = glm::vec2(xpos, ypos);
        viewOffset.x -= delta.x / currentZoomLevel;
        viewOffset.y += delta.y / currentZoomLevel;
    }
}

void setupImGUI(GLFWwindow *window){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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

        glm::vec3 clickCenter(world_click_center_pos.x, world_click_center_pos.y, 0.1f);
        // Promień rozrzutu 
        float radius = 2.0f; 

        for (int i = 0; i < bacteriaCount; ++i){
            glm::vec2 randomOffset = glm::gaussRand(glm::vec2(0.0f), glm::vec2(radius));
            float offsetZ = static_cast<float>(i) * 0.01f;
            glm::vec3 spawnPosition = clickCenter + glm::vec3(randomOffset.x, randomOffset.y, offsetZ);

            const float maxZ = 2.0f;
            if (spawnPosition.z > maxZ) spawnPosition.z = maxZ;

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
        renderer.setLightRange(range*1.2f);
        renderer.setGlowRadius(range*1.2f);
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
    glfwSetKeyCallback(renderer.getWindow(), key_callback);
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

        // === sekcja UPDATE ===
        // co klatkę aktualizujemy divisionTimer bakterii oraz sprawdzamy, czy nadaje się do usunięcia
        // a także co klatkę aktualizujemy działanie antybiotyku na bakterie
        for (auto& bacteria : allBacteria) {
            if (bacteria) {
                bacteria->update(deltaTime);
            }
        }
        
        // rozmnażanie bakterii
        std::vector<std::unique_ptr<IBacteria>> newBacteria;
        for (auto& bacteria : allBacteria) {
            if (bacteria && bacteria->canDivide()) { 
                if (glm::linearRand(0.0f, 1.0f) < 0.05f) { // 5% szansy na podział
                    IBacteria* child = bacteria->clone();
                    if (child) {
                        newBacteria.push_back(std::unique_ptr<IBacteria>(child));
                    }
                }
                bacteria->resetDivisionTimer();
            }
        }

        for (auto& child : newBacteria) {
            allBacteria.push_back(std::move(child));
        }
        
        // usuwanie bakterii
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
        guiRenderer.render(viewOffset, currentZoomLevel, WINDOW_HEIGHT, is3DView);  // Logika UI w ImGui

        // Rozpoczęcie klatki renderowania sceny
        renderer.beginFrame();

        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;

        if (is3DView){
            projectionMatrix = glm::perspective(glm::radians(45.0f),
            static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.0f);
            glm::vec3 offset(cos(cameraPitch) * sin(cameraYaw),
                             cos(cameraPitch) * cos(cameraYaw),
                             sin(cameraPitch));
            glm::vec3 cameraPos = cameraTarget3D + cameraDistance * offset;
            viewMatrix = glm::lookAt(cameraPos, cameraTarget3D, cameraUp3D);     
        }
        else{
            // Obliczanie szerokości i wysokości widoku w jednostkach świata
            float viewW = WINDOW_WIDTH / currentZoomLevel;
            float viewH = WINDOW_HEIGHT / currentZoomLevel;

            // Definiowanie rogów szalki (mapy świata) - projekcia ortogonalna
            projectionMatrix = glm::ortho(viewOffset.x, viewOffset.x + viewW,
                                          viewOffset.y, viewOffset.y + viewH, -1.0f, 1.0f);
            viewMatrix = glm::mat4(1.0f);
        }


        // Poniższa macierz będzie przekazana do renderera i do shaderów
        glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

        if (is3DView) {
            renderer.renderPetriDish(viewProjectionMatrix, viewMatrix); 
        }
        // Renderowanie kolonii bakterii
        renderer.renderColony(allBacteria, currentZoomLevel, viewProjectionMatrix); 

        //Renderowanie antybiotyków:
        renderer.renderAntibioticEffects(viewProjectionMatrix);

        // Renderowanie poświaty 
        if (!is3DView)
        renderer.renderGlowEffect(projectionMatrix, viewMatrix, currentZoomLevel);

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