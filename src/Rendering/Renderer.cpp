#include "Renderer.h" 
#include "../Simulation/IBacteria.h" 

// Inicjalizacje procedur, obslugi klawiatury, bledow, itd - wszystko to, co wykonujemy raz na początku programu
void Renderer::setupInitialProcedures() {
    // Ustawienie ortograficznej macierzy projekcji
    // Mapuje współrzędne świata (0,0) do (WINDOW_WIDTH, WINDOW_HEIGHT) na okno
    // (0,0) jest w lewym górnym rogu
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//Inicjalizacja środowiska OpenGL oraz okna programu
void Renderer::initOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Tworzymy okno i kontekst OpenGL
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PetriDish Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    setupInitialProcedures();
}

Renderer::Renderer() : window(nullptr) {
    // Inicjalizacja OpenGL, shaderów itp.
    initOpenGL();
}

Renderer::~Renderer() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Zwracanie wskaźnika do okna
GLFWwindow* Renderer::getWindow() const {
    return window;
}

// Metoda do renderowania pojedynczej bakterii w widoku mikroskopowym
void Renderer::renderBacteria(IBacteria& bacteria, int gridWidth, int gridHeight) {
    // Pozycja bakterii w systemie siatki
    int gridX = bacteria.getX();
    int gridY = bacteria.getY();

    float renderX = static_cast<float>(gridX) * (static_cast<float>(WINDOW_WIDTH) / gridWidth) + (static_cast<float>(WINDOW_WIDTH) / gridWidth) / 2.0f;
    float renderY = static_cast<float>(gridY) * (static_cast<float>(WINDOW_HEIGHT) / gridHeight) + (static_cast<float>(WINDOW_HEIGHT) / gridHeight) / 2.0f;

    float health = bacteria.getHealth();
    float red = 1.0f - health; 
    float green = health;        
    float blue = 0.0f; 

    glColor3f(red, green, blue); 

    const auto& circuit = bacteria.getCircuit();

    if (!circuit.empty()) {
        glBegin(GL_POLYGON); 
        for (const auto& [dx, dy] : circuit) {
             glVertex2f(renderX + dx, renderY + dy);
        }
        glEnd();
    }
}

void Renderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    glfwSwapBuffers(window);
}
