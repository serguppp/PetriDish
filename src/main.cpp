#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Simulation/Coccus.cpp"
#include "Rendering/Renderer.cpp"
#include "Simulation/Coccus.cpp"

int main() {
    //***********************************************************************
    //Tutaj tworzymy nasze obiekty
    // Tworzymy obiekt Renderer, który będzie odpowiadał za renderowanie okna OpenGL
    Renderer renderer;

    // Tworzymy bakterię na pozycji (100, 100)
    Coccus bacteria(100, 100);
    //***********************************************************************

    // Główna pętla programu
    while (!glfwWindowShouldClose(renderer.getWindow())) {
        // Przetwarzamy zdarzenia (np. wejście z klawiatury)
        glfwPollEvents();

        //***********************************************************************
        //Tutaj możemy wykonywać operacje na naszych obiektach: renderowanie, modyfikacja pozycji, zdrowia itd

        // Renderujemy bakterie
        renderer.render(bacteria);

        // Zwiększamy timer bakterii
        bacteria.update(0.1f);

        // Zmiana zdrowia bakterii w zależności od czasu (przykład)
        if (bacteria.getHealth() > 0.0f) {
            bacteria.applyAntibiotic(0.01f); // Aplikujemy antybiotyk co klatkę
        }
        //***********************************************************************
    }

    // to przeniesc do renderera
	//freeOpenGLProgram(window);
	//glfwDestroyWindow(window); 
    glfwTerminate();
    return 0;
}
