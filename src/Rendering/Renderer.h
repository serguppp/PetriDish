#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include "../Simulation/IBacteria.h" 

#include <iostream>
#include <vector>
#include <utility>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

class Renderer {
private:
    GLFWwindow* window;

    // Inicjalizacje procedur - wszystko to, co wykonujemy raz na początku programu
    void setupInitialProcedures();

    //Inicjalizacja środowiska OpenGL oraz okna programu
    void initOpenGL();

public:
    Renderer(); 

    ~Renderer();

    // Zwracanie wskaźnika do okna
    GLFWwindow* getWindow() const;

    // Główna procedura odpowiadająca za renderowanie klatki w pętli
    void beginFrame();
    
    void endFrame();

    void renderBacteria(IBacteria& bacteria); 

    void renderColony(std::vector<std::unique_ptr<IBacteria>>& allBacteria);

};
