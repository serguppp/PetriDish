#include "Renderer.h"
#include "glm/gtc/matrix_transform.hpp" 
#include "glm/gtc/type_ptr.hpp"    
#include <vector>                    

const float MICROSCOPIC_VIEW_THRESHOLD = 1.5f;
const float BACTERIA_MODEL_SCALE_FACTOR = 0.5f; 

Renderer::Renderer(int width, int height)
    : window(nullptr), windowWidth(width), windowHeight(height), successfullyInitialized(false) {
    successfullyInitialized = initOpenGL(width, height);
}

Renderer::~Renderer() {
    if (window) {
        glfwDestroyWindow(window);
    }
}

bool Renderer::initOpenGL(int width, int height) {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "PetriDish Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window in Renderer" << std::endl;
        return false; 
    }
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW in Renderer: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window); 
        window = nullptr;
        return false; 
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    setupInitialProcedures();
    glfwSwapInterval(1);
    return true; 
}

bool Renderer::isInitialized() const {
    return successfullyInitialized;
}

void Renderer::setupInitialProcedures() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
}

GLFWwindow* Renderer::getWindow() const {
    return window;
}

void Renderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    if (window) {
        glfwSwapBuffers(window);
    }
}

void Renderer::renderBacteria(IBacteria& bacteria, float zoomLevel) {
    if (!bacteria.isAlive()) return;

    glm::vec4 position = bacteria.getPos();
    BacteriaType type = bacteria.getBacteriaType();

    switch (type) {
        case BacteriaType::Cocci: glColor3f(0.9f, 0.4f, 0.4f); break;
        case BacteriaType::Diplococcus: glColor3f(0.4f, 0.9f, 0.4f); break;
        case BacteriaType::Staphylococci: glColor3f(0.4f, 0.4f, 0.9f); break;
        default: glColor3f(0.7f, 0.7f, 0.7f); break;
    }

    if (zoomLevel < MICROSCOPIC_VIEW_THRESHOLD) {
        // Widok makro: kolonia bakterii jako punkty
        float pointSize = 3.0f; 
        glPointSize(pointSize);


        glBegin(GL_POINTS);
        glVertex2f(position.x, position.y);
        glEnd();
    } else {
        // Widok mikro: bakterie jako skalowalne modele
        glPushMatrix();
        glTranslatef(position.x, position.y, 0.0f);

        float modelScale = BACTERIA_MODEL_SCALE_FACTOR;
        glScalef(modelScale, modelScale, 1.0f);

        const auto& circuit = bacteria.getCircuit();
        if (!circuit.empty()) {
            float health = bacteria.getHealth(); 
            GLfloat currentColor[4];
            glGetFloatv(GL_CURRENT_COLOR, currentColor);
            // Przyciemnianie koloru w zależności od HP
            glColor4f(currentColor[0] * health, currentColor[1] * health, currentColor[2] * health, currentColor[3] * (health > 0.1f ? 1.0f : health * 2.0f) ); // Zadbaj o alpha

            glBegin(GL_POLYGON);
            for (const auto& vertexOffset : circuit) {
                glVertex2f(static_cast<float>(vertexOffset.first), static_cast<float>(vertexOffset.second));
            }
            glEnd();
        } 
        glPopMatrix();
    }
}

void Renderer::renderColony(const std::vector<std::unique_ptr<IBacteria>>& allBacteria, float zoomLevel) {
    for (const auto& bacteriaPtr : allBacteria) {
        if (bacteriaPtr) {
            renderBacteria(*bacteriaPtr, zoomLevel);
        }
    }
}

void Renderer::addAntibioticEffect(const glm::vec2& worldPos, float strength, float radius, float lifetime) {
    activeAntibiotics.push_back({worldPos, strength, radius, 0.0f, lifetime});
}

void Renderer::updateAntibioticEffects(float deltaTime) {
    for (auto& antibiotic : activeAntibiotics) {
        antibiotic.timeApplied += deltaTime;
    }
    activeAntibiotics.erase(
        std::remove_if(activeAntibiotics.begin(), activeAntibiotics.end(), 
                       [](const AntibioticEffect& effect) {
                           return effect.timeApplied >= effect.maxLifetime;
                       }),
        activeAntibiotics.end()
    );
}

void Renderer::renderAntibioticEffects(float zoomLevel) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    for (const auto& antibiotic : activeAntibiotics) {
        float effectProgress = antibiotic.timeApplied / antibiotic.maxLifetime;
        float currentRadius = antibiotic.radius * (1.0f - effectProgress); // Kurczenie się poświaty
        float alpha = 1.0f - effectProgress;

        if (alpha <= 0.0f || currentRadius <= 0.0f) continue;

        glColor4f(0.5f, 0.7f, 1.0f, alpha * 0.5f); 

        glPushMatrix();
        glTranslatef(antibiotic.worldPosition.x, antibiotic.worldPosition.y, 0.0f);
        
        const int num_segments = 50;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f);
        for (int i = 0; i <= num_segments; i++) {
            float angle = 2.0f * 3.1415926f * static_cast<float>(i) / static_cast<float>(num_segments);
            float x = currentRadius * cosf(angle);
            float y = currentRadius * sinf(angle);
            glVertex2f(x, y);
        }
        glEnd();
        glPopMatrix();
    }
    glDisable(GL_BLEND);
}