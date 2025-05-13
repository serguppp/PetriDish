#include "Renderer.h" 

Renderer::Renderer() : window(nullptr) {
    // Inicjalizacja OpenGL, shaderów itp.
    initOpenGL();
}

// Inicjalizacje procedur - wszystko to, co wykonujemy raz na początku programu
void Renderer::setupInitialProcedures() {
    // Ustawienie ortograficznej macierzy projekcji
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

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
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

// Zwracanie wskaźnika do okna
GLFWwindow* Renderer::getWindow() const {
    return window;
}

void Renderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    glfwSwapBuffers(window);
}

Renderer::~Renderer() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Metoda do renderowania pojedynczej bakterii
void Renderer::renderBacteria(IBacteria& bacteria) {
    glm::vec4 position = bacteria.getPos();
    BacteriaType type = bacteria.getBacteriaType(); 

    switch (type) {
        case BacteriaType::Cocci: glColor3f(0.9f, 0.4f, 0.4f); break; // czerwony
        case BacteriaType::Diplococcus: glColor3f(0.4f, 0.9f, 0.4f); break; // zielony
        case BacteriaType::Staphylococci: glColor3f(0.4f, 0.4f, 0.9f); break; // niebieski
        default: glColor3f(0.7f, 0.7f, 0.7f); break;
    }
    glBegin(GL_POINTS);
    glVertex2f(position.x, position.y);
    glEnd();

    /*
    float health = bacteria.getHealth();
    float red = 1.0f - health; 
    float green = health;        
    float blue = 0.0f; 

    glColor3f(red, green, blue); 

    const auto& circuit = bacteria.getCircuit();
    
    if (!circuit.empty()) {
        glBegin(GL_POLYGON); 
        for (const auto& [dx, dy] : circuit) {
             glVertex2f(position.x + dx, position.y + dy);
        }
        glEnd();
    }
    */
}

void Renderer::renderColony(std::vector<std::unique_ptr<IBacteria>>& allBacteria) {
    for (auto& bacteria : allBacteria) {
        if (bacteria && bacteria->isAlive())
            renderBacteria(*bacteria);
    }
}