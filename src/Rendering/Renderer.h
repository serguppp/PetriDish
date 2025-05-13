#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>
#include <iostream> 

#include "Simulation/IBacteria.h" 
#include "Simulation/AntibioticEffect.h"

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    GLFWwindow* getWindow() const;
    bool isInitialized() const; 

    void beginFrame();
    void endFrame();

    void renderBacteria(IBacteria& bacteria, float zoomLevel);
    void renderColony(const std::vector<std::unique_ptr<IBacteria>>& allBacteria, float zoomLevel);

    void addAntibioticEffect(const glm::vec2& worldPos, float strength, float radius, float lifetime = 2.0f);
    void updateAntibioticEffects(float deltaTime);
    void renderAntibioticEffects(float zoomLevel);

private:
    bool initOpenGL(int width, int height); 
    void setupInitialProcedures();

    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    bool successfullyInitialized; 

    std::vector<AntibioticEffect> activeAntibiotics; 
};