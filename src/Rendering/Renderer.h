#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <iostream> 
#include "Simulation/IBacteria.h" 

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
    void renderAntibiotic();

private:
    bool initOpenGL(int width, int height); 
    void setupInitialProcedures();

    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    bool successfullyInitialized; 
};