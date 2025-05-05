#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../Simulation/Coccus.cpp" 
#include <iostream>

class Renderer {
private:
    GLFWwindow* window;

    //***********************************************************************
    // Inicjalizacje procedur, obslugi klawiatury, bledow, itd - wszystko to, co wykonujemy raz na początku programu
    void setupInitialProcedures() {
        glOrtho(0, 800, 600, 0, -1, 1);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  
    }
    //***********************************************************************

    //Inicjalizacja środowiska OpenGL oraz okna programu
    void initOpenGL() {
        // Inicjalizacja GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Tworzymy okno i kontekst OpenGL
        window = glfwCreateWindow(800, 600, "PetriDish Simulation", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to open GLFW window" << std::endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        // ??
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); 

        // Inicjalizacja GLEW
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            exit(EXIT_FAILURE);
        }  

        //Inicjalizujemy początkowe procedury
        setupInitialProcedures();
    }
public:
    Renderer() {
        // Inicjalizacja OpenGL, shaderów itp.
        initOpenGL();
    }

    // Zwracanie wskaźnika do okna
    GLFWwindow* getWindow() const {
        return window;  
    }

    //***********************************************************************
    //Tutaj umieszczamy kod odpowiedzialny za logikę rysowania naszych obiektów: pozycję, zmiany tekstury itd
    void renderBacteria(const Coccus& bacteria) {
        // Pozycja bakterii
        int x = bacteria.getX();
        int y = bacteria.getY();

        // Kolor bakterii (na przykład czerwony, można zmieniać w zależności od zdrowia)
        float red = bacteria.getHealth();  // Kolor bakterii zmienia się w zależności od jej zdrowia
        float green = 1.0f - bacteria.getHealth();

        glColor3f(red, green, 0.0f);  // Ustaw kolor na podstawie zdrowia

        // Narysowanie kwadratu reprezentującego bakterię
        glBegin(GL_QUADS);
            glVertex2f(x - 5, y - 5);
            glVertex2f(x + 5, y - 5);
            glVertex2f(x + 5, y + 5);
            glVertex2f(x - 5, y + 5);
        glEnd();
    }

    //***********************************************************************

    //Główna procedura odpowiadająca za renderowanie klatki w pętli
    void render(const Coccus& bacteria) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Czyszczenie bufora kolorów

        //***********************************************************************
        //Tutaj zajmujemy się rysowaniem zawartości sceny: wywolujemy metody odpowiadajace za poszczegolne obiekty
        renderBacteria(bacteria);
        //***********************************************************************
        // Swap buffers, aby pokazać to, co zostało narysowane na ekranie
        glfwSwapBuffers(window);
    }
};
