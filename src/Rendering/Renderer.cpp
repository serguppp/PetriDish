#include "Renderer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Próg zoomu decydujący o przełączeniu między widokiem makro (punkty) a mikro (modele bakterii)
const float MICROSCOPIC_VIEW_THRESHOLD = 1.5f; 
// Współczynnik skalowania modeli bakterii w widoku mikro
const float BACTERIA_MODEL_SCALE_FACTOR = 0.5f; 

Renderer::Renderer(int width, int height)
    : window(nullptr), windowWidth(width), windowHeight(height), successfullyInitialized(false),
      lightPosWorld(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f, 10.0f), 
      lightColor(1.5f, 1.5f, 1.5f),         
      ambientColor(0.5f, 0.5f, 0.5f), 
      lightRange(100.0f), 
      lightIntensity(1.0f), 
      glowEffectColor(1.0f, 0.9f, 0.7f), 
      glowEffectRadius(100.0f),         
      glowEffectIntensityFactor(0.75f)   
{
    successfullyInitialized = initOpenGL(width, height);
    if (successfullyInitialized) {
        // Inicjalizacja shaderów po pomyślnym utworzeniu kontekstu OpenGL
        initBacteriaShader();
        setupBacteriaGeometry();

        initPointShader();
        setupPointGeometry();

        initAntibioticShader();
        setupAntibioticGeometry();
        
        initGlowShader();     
        setupGlowGeometry();  
    }
}

Renderer::~Renderer() {
    // Czyszczenie zasobów VAO i VBO dla bakterii
    for (auto const& [type, vaoID] : bacteriaVAOs) {
        if (vaoID != 0) glDeleteVertexArrays(1, &vaoID);
    }
    bacteriaVAOs.clear();

    for (auto const& [type, vboID] : bacteriaVBOs_vertexLocalPosition) { 
        if (vboID != 0) glDeleteBuffers(1, &vboID);
    }
    bacteriaVBOs_vertexLocalPosition.clear(); 
    bacteriaVertexCounts.clear(); 

    // Czyszczenie zasobów dla punktów, antybiotyków i poświaty
    if (pointVAO != 0) glDeleteVertexArrays(1, &pointVAO);
    if (pointVBO_vertexPosition != 0) glDeleteBuffers(1, &pointVBO_vertexPosition); 
    if (antibioticCircleVAO != 0) glDeleteVertexArrays(1, &antibioticCircleVAO);
    if (antibioticCircleVBO_vertexPosition != 0) glDeleteBuffers(1, &antibioticCircleVBO_vertexPosition);
    if (glowVAO != 0) glDeleteVertexArrays(1, &glowVAO);
    if (glowVBO_vertexPosition != 0) glDeleteBuffers(1, &glowVBO_vertexPosition);

    if (!activeAntibiotics.empty()) {
         activeAntibiotics.clear();
    }

    if (window) {
        glfwDestroyWindow(window);
    }
}

bool Renderer::initOpenGL(int width, int height) {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Okno nie będzie miało zmiennego rozmiaru

    // Wymaganie OpenGL w wersji 3.0 lub nowszej
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0); 

    // Tworzenie okna GLFW
    window = glfwCreateWindow(width, height, "Symulacja Szalki Petriego", nullptr, nullptr);
    if (!window) {
        std::cerr << "Błąd: Nie udało się utworzyć okna GLFW w Rendererze" << std::endl;
        const char* description;
        glfwGetError(&description);
        if (description) {
            std::cerr << "Błąd GLFW: " << description << std::endl;
        }
        return false;
    }
    glfwMakeContextCurrent(window); 

    // Inicjalizacja GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Błąd: Nie udało się zainicjalizować GLEW w Rendererze: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        window = nullptr;
        return false;
    }

    // Wyświetlenie informacji o wersji OpenGL i GLSL
    std::cout << "Wersja OpenGL: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Wersja GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Ustawienie początkowych procedur renderowania
    setupInitialProcedures();
    glfwSwapInterval(1); // Włączenie V-Sync
    return true;
}

bool Renderer::isInitialized() const {
    return successfullyInitialized;
}

void Renderer::setupInitialProcedures() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // Ustawienie koloru tła
    glEnable(GL_BLEND); // Włączenie mieszania kolorów (blending)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Standardowe ustawienie blendingu dla przezroczystości
    glEnable(GL_DEPTH_TEST);  // Włączenie testu głębi (Z-bufor)
}

GLFWwindow* Renderer::getWindow() const {
    return window;
}

void Renderer::beginFrame() {
    // Czyszczenie bufora koloru i głębi na początku każdej klatki
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    if (window) 
        glfwSwapBuffers(window); // Zamiana buforów (przedni z tylnym)
}

// Inicjalizacja shadera bakterii
void Renderer::initBacteriaShader() {
    bacteriaShaderProgramID = shaderManager.loadShaderProgram("bacteriaShader", "shaders/bacteria.vert", "shaders/bacteria.frag");

    if (bacteriaShaderProgramID == 0) {
        std::cerr << "Renderer: Błąd ładowania programu shadera bakterii! Renderowanie może nie działać poprawnie." << std::endl;
        return;
    }

    // Pobieranie lokalizacji uniformów z shadera bakterii
    bacteria_u_viewProjectionMatrix_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_viewProjectionMatrix");
    bacteria_u_instanceWorldPosition_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_instanceWorldPosition");
    bacteria_u_instanceScale_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_instanceScale");
    bacteria_u_bacteriaType_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_bacteriaType");
    bacteria_u_bacteriaHealth_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_bacteriaHealth");
    bacteria_u_time_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_time");

    bacteria_u_lightPositionWorld_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_lightPositionWorld");
    bacteria_u_lightColor_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_lightColor");
    bacteria_u_ambientColor_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_ambientColor");
    bacteria_u_cameraPositionWorld_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_cameraPositionWorld");
    bacteria_u_lightRange_loc = shaderManager.getUniformLocation(bacteriaShaderProgramID, "u_lightRange");

}

// Ustawienie geometrii bakterii
void Renderer::setupBacteriaGeometry() {
    bacteriaShaderProgramID = shaderManager.getShaderProgram("bacteriaShader"); 
    if (bacteriaShaderProgramID == 0) {
        std::cerr << "Renderer: Nie można ustawić geometrii bakterii, program shadera niezaładowany." << std::endl;
        return;
    }

    for (int i = 0; i <= static_cast<int>(BacteriaType::Bacillus); ++i) { 
        BacteriaType type = static_cast<BacteriaType>(i);
        BacteriaStats stats = getStatsForType(type); 

        if (stats.circuit.empty()) {
            std::cout << "Renderer: Obwód dla typu bakterii " << i << " jest pusty." << std::endl;
            continue;
        }

        std::vector<glm::vec2> vertices;
        for (const auto& p : stats.circuit) {
            vertices.push_back(glm::vec2(p.first, p.second));
        }
        
        if (vertices.empty()) { 
             std::cout << "Renderer: Dane wierzchołków dla typu bakterii " << i << " są puste po przetworzeniu obwodu." << std::endl;
            continue;
        }
        bacteriaVertexCounts[type] = static_cast<int>(vertices.size());

        GLuint vao_id, vbo_pos_id;
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(1, &vbo_pos_id);

        glBindVertexArray(vao_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_id);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

        GLint posAttribLoc = glGetAttribLocation(bacteriaShaderProgramID, "a_vertexLocalPosition");
        if (posAttribLoc != -1) {
            glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
            glEnableVertexAttribArray(posAttribLoc);
        } else {
             std::cerr << "Renderer: Atrybut a_vertexLocalPosition nie znaleziony w bacteriaShader podczas ustawiania geometrii." << std::endl;
        }

        bacteriaVAOs[type] = vao_id;
        bacteriaVBOs_vertexLocalPosition[type] = vbo_pos_id; 

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0);            
    }
     std::cout << "Renderer: Ustawienie geometrii bakterii zakończone." << std::endl;
}

// Inicjalizacja shadera punktów
void Renderer::initPointShader() {
    pointShaderProgramID = shaderManager.loadShaderProgram("pointShader", "shaders/point.vert", "shaders/point.frag");
    if (pointShaderProgramID == 0) {
        std::cerr << "Renderer: Błąd ładowania programu shadera punktów!" << std::endl;
        return;
    }
    point_u_modelMatrix_loc = shaderManager.getUniformLocation(pointShaderProgramID, "u_modelMatrix");
    point_u_viewProjectionMatrix_loc = shaderManager.getUniformLocation(pointShaderProgramID, "u_viewProjectionMatrix");
    point_u_renderPointSize_loc = shaderManager.getUniformLocation(pointShaderProgramID, "u_renderPointSize");
    point_u_pointColor_loc = shaderManager.getUniformLocation(pointShaderProgramID, "u_pointColor");
}

// Ustawienie geometrii dla punktów
void Renderer::setupPointGeometry() {
    float point_vertex[] = { 0.0f, 0.0f }; 
    glGenVertexArrays(1, &pointVAO);
    glGenBuffers(1, &pointVBO_vertexPosition); 
    glBindVertexArray(pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO_vertexPosition); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(point_vertex), point_vertex, GL_STATIC_DRAW);
    
    GLint posAttribLoc = glGetAttribLocation(pointShaderProgramID, "a_vertexPosition");
     if (posAttribLoc != -1) {
        glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(posAttribLoc);
    } else {
        std::cerr << "Renderer: Atrybut a_vertexPosition nie znaleziony w pointShader." << std::endl;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Renderowanie pojedynczej bakterii
void Renderer::renderBacteria(IBacteria& bacteria, float zoomLevel, const glm::mat4& viewProjectionMatrix) {
    if (!bacteria.isAlive()) return;

    glm::vec4 posVec4 = bacteria.getPos();
    BacteriaType type = bacteria.getBacteriaType();
    glm::vec3 bacteriaColor;

    switch (type) {
        case BacteriaType::Cocci: bacteriaColor = glm::vec3(0.9f, 0.4f, 0.4f); break;
        case BacteriaType::Diplococcus: bacteriaColor = glm::vec3(0.4f, 0.9f, 0.4f); break;
        case BacteriaType::Staphylococci: bacteriaColor = glm::vec3(0.4f, 0.4f, 0.9f); break;
        case BacteriaType::Bacillus: bacteriaColor = glm::vec3(0.8f, 0.6f, 0.2f); break;
        default: bacteriaColor = glm::vec3(0.7f, 0.7f, 0.7f); break;
    }

    if (zoomLevel < MICROSCOPIC_VIEW_THRESHOLD) {
        // Widok makro: renderowanie jako punkty
        if (pointShaderProgramID == 0 || pointVAO == 0) return;
        shaderManager.useShaderProgram(pointShaderProgramID);
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(posVec4.x, posVec4.y, 0.0f));
        
        glUniformMatrix4fv(point_u_modelMatrix_loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(point_u_viewProjectionMatrix_loc, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
        glUniform1f(point_u_renderPointSize_loc, 3.0f); // Stały rozmiar punktu w widoku makro
        glUniform3fv(point_u_pointColor_loc, 1, glm::value_ptr(bacteriaColor * bacteria.getHealth())); // Kolor zależny od zdrowia

        glBindVertexArray(pointVAO);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);

    } else {
        // Widok mikro: renderowanie pełnego modelu bakterii
        if (bacteriaShaderProgramID == 0 || bacteriaVAOs.find(type) == bacteriaVAOs.end()) return;

        shaderManager.useShaderProgram(bacteriaShaderProgramID);

        // Ustawianie uniformów dla shadera bakterii
        glUniformMatrix4fv(bacteria_u_viewProjectionMatrix_loc, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
        glUniform2f(bacteria_u_instanceWorldPosition_loc, posVec4.x, posVec4.y);
        glUniform1f(bacteria_u_instanceScale_loc, BACTERIA_MODEL_SCALE_FACTOR);
        glUniform1i(bacteria_u_bacteriaType_loc, static_cast<int>(type)); 
        glUniform1f(bacteria_u_bacteriaHealth_loc, bacteria.getHealth()); 
        glUniform1f(bacteria_u_time_loc, static_cast<float>(glfwGetTime()));

        // Uniformy oświetlenia
        glUniform3fv(bacteria_u_lightPositionWorld_loc, 1, glm::value_ptr(lightPosWorld));
        glUniform3fv(bacteria_u_lightColor_loc, 1, glm::value_ptr(lightColor));
        glUniform3fv(bacteria_u_ambientColor_loc, 1, glm::value_ptr(ambientColor));
        
        glm::vec3 cameraPosWorld(viewProjectionMatrix[3][0], viewProjectionMatrix[3][1], 100.0f); 
        glUniform3fv(bacteria_u_cameraPositionWorld_loc, 1, glm::value_ptr(cameraPosWorld));
        glUniform1f(bacteria_u_lightRange_loc, lightRange);

        // Renderowanie geometrii bakterii
        auto vao_it = bacteriaVAOs.find(type);
        if (vao_it != bacteriaVAOs.end() && bacteriaVertexCounts.count(type) && bacteriaVertexCounts.at(type) > 0) {
            glBindVertexArray(vao_it->second);
            glDrawArrays(GL_TRIANGLE_FAN, 0, bacteriaVertexCounts.at(type));
            glBindVertexArray(0);
        }
    }
    shaderManager.useShaderProgram(0); 
}

// Renderowanie całej kolonii bakterii
void Renderer::renderColony(const std::vector<std::unique_ptr<IBacteria>>& allBacteria, float zoomLevel, const glm::mat4& viewProjectionMatrix) {
    for (auto it = allBacteria.rbegin(); it != allBacteria.rend(); ++it) {
        const std::unique_ptr<IBacteria>& bacteriaPtr = *it;
        if (bacteriaPtr) { 
            renderBacteria(*bacteriaPtr, zoomLevel, viewProjectionMatrix);
        }
    }
}

// Dodawanie efektu antybiotyku
void Renderer::addAntibioticEffect(const glm::vec2& worldPos, float strength, float radius, float lifetime) {
    activeAntibiotics.push_back({worldPos, strength, radius, 0.0f, lifetime});
}

// Aktualizacja efektów antybiotyków 
void Renderer::updateAntibioticEffects(float deltaTime) {
    for (auto& antibiotic : activeAntibiotics) {
        antibiotic.timeApplied += deltaTime;
    }
    // Usuwanie przestarzałych efektów
    activeAntibiotics.erase(
        std::remove_if(activeAntibiotics.begin(), activeAntibiotics.end(),
                       [](const AntibioticEffect& effect) {
                           return effect.timeApplied >= effect.maxLifetime;
                       }),
        activeAntibiotics.end()
    );
}

// Renderowanie efektów antybiotyków
void Renderer::renderAntibioticEffects(const glm::mat4& viewProjectionMatrix) {
    if (antibioticShaderProgramID == 0 || antibioticCircleVAO == 0) return;

    shaderManager.useShaderProgram(antibioticShaderProgramID);
    glUniformMatrix4fv(antibiotic_u_viewProjectionMatrix_loc, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

    glEnable(GL_BLEND); // Włączenie blendingu dla efektu przezroczystości
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(antibioticCircleVAO);

    for (const auto& antibiotic : activeAntibiotics) {
        float effectProgress = antibiotic.timeApplied / antibiotic.maxLifetime;
        float currentRadius = antibiotic.radius * (1.0f - effectProgress); // Efekt kurczenia się
        float alpha = (1.0f - effectProgress) * 0.5f; // Zanikanie efektu

        if (alpha <= 0.0f || currentRadius <= 0.0f) continue; 

        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(antibiotic.worldPosition, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(currentRadius, currentRadius, 1.0f));

        glUniformMatrix4fv(antibiotic_u_modelMatrix_loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniform4f(antibiotic_u_effectColor_loc, 0.5f, 0.7f, 1.0f, alpha); // Kolor z przezroczystością

        glDrawArrays(GL_TRIANGLE_FAN, 0, antibioticCircleVertexCount);
    }
    glBindVertexArray(0);
    glDisable(GL_BLEND); 
    shaderManager.useShaderProgram(0);
}

// Inicjalizacja shadera antybiotyków
void Renderer::initAntibioticShader() {
    antibioticShaderProgramID = shaderManager.loadShaderProgram("antibioticShader", "shaders/antibiotic.vert", "shaders/antibiotic.frag");
    if (antibioticShaderProgramID == 0) {
        std::cerr << "Renderer: Błąd ładowania programu shadera antybiotyków!" << std::endl;
        return;
    }
    antibiotic_u_modelMatrix_loc = shaderManager.getUniformLocation(antibioticShaderProgramID, "u_modelMatrix");
    antibiotic_u_viewProjectionMatrix_loc = shaderManager.getUniformLocation(antibioticShaderProgramID, "u_viewProjectionMatrix");
    antibiotic_u_effectColor_loc = shaderManager.getUniformLocation(antibioticShaderProgramID, "u_effectColor");
}

// Ustawienie geometrii dla efektu antybiotyku (koło)
void Renderer::setupAntibioticGeometry() {
    const int num_segments = 50; 
    std::vector<glm::vec2> circleVertices;
    circleVertices.push_back(glm::vec2(0.0f, 0.0f)); 

    for (int i = 0; i <= num_segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(num_segments);
        circleVertices.push_back(glm::vec2(cosf(angle), sinf(angle))); 
    }
    antibioticCircleVertexCount = static_cast<int>(circleVertices.size());

    glGenVertexArrays(1, &antibioticCircleVAO);
    glGenBuffers(1, &antibioticCircleVBO_vertexPosition); 

    glBindVertexArray(antibioticCircleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, antibioticCircleVBO_vertexPosition); 
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(glm::vec2), circleVertices.data(), GL_STATIC_DRAW);

    GLint posAttribLoc = glGetAttribLocation(antibioticShaderProgramID, "a_vertexPosition");
    if (posAttribLoc != -1) {
        glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(posAttribLoc);
    } else {
        std::cerr << "Renderer: Atrybut a_vertexPosition nie znaleziony w antibioticShader." << std::endl;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Inicjalizacja shadera poświaty
void Renderer::initGlowShader() {
    glowShaderProgramID = shaderManager.loadShaderProgram("glowShader", "shaders/glow.vert", "shaders/glow.frag");
    if (glowShaderProgramID == 0) {
        std::cerr << "Renderer: Błąd ładowania programu shadera poświaty!" << std::endl;
        return;
    }

    glow_u_lightScreenPosition_loc = shaderManager.getUniformLocation(glowShaderProgramID, "u_lightScreenPosition");
    glow_u_screenResolution_loc = shaderManager.getUniformLocation(glowShaderProgramID, "u_screenResolution");
    glow_u_glowEffectColor_loc = shaderManager.getUniformLocation(glowShaderProgramID, "u_glowEffectColor");
    glow_u_glowEffectRadius_loc = shaderManager.getUniformLocation(glowShaderProgramID, "u_glowEffectRadius");
    glow_u_glowEffectIntensity_loc = shaderManager.getUniformLocation(glowShaderProgramID, "u_glowEffectIntensity");
}

// Ustawienie geometrii dla efektu poświaty 
void Renderer::setupGlowGeometry() {
    // Wierzchołki kwadratu pokrywającego cały ekran w przestrzeni
    float quadVertices[] = {
        // pozycje
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &glowVAO);
    glGenBuffers(1, &glowVBO_vertexPosition); 
    glBindVertexArray(glowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, glowVBO_vertexPosition); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    
    GLint posAttribLoc = glGetAttribLocation(glowShaderProgramID, "a_vertexPosition"); 
    if (posAttribLoc != -1) {
        glEnableVertexAttribArray(posAttribLoc);
        glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    } else {
         std::cerr << "Renderer: Atrybut a_vertexPosition nie znaleziony w glowShader." << std::endl;
    }
    glBindVertexArray(0);
}

// Renderowanie efektu poświaty
void Renderer::renderGlowEffect(const glm::mat4& projection, const glm::mat4& view, float currentZoomLevel) {
    if (glowShaderProgramID == 0 || glowVAO == 0) return;
    shaderManager.useShaderProgram(glowShaderProgramID);

    // Transformacja pozycji światła ze świata do przestrzeni ekranu 
    glm::vec4 lightPosClip = projection * view * glm::vec4(lightPosWorld, 1.0);
    glm::vec3 lightPosNDC = glm::vec3(lightPosClip.x / lightPosClip.w, lightPosClip.y / lightPosClip.w, lightPosClip.z / lightPosClip.w);
    glm::vec2 lightPosScreen(
        (lightPosNDC.x * 0.5f + 0.5f) * windowWidth,
        (lightPosNDC.y * 0.5f + 0.5f) * windowHeight
    );

    glUniform2fv(glow_u_lightScreenPosition_loc, 1, glm::value_ptr(lightPosScreen));
    glUniform2f(glow_u_screenResolution_loc, static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    glUniform3fv(glow_u_glowEffectColor_loc, 1, glm::value_ptr(glowEffectColor));

    // Promień poświaty w pikselach, skalowany przez zoom dla spójnego wyglądu
    float glowRadiusPixels = glowEffectRadius * currentZoomLevel; 
    glUniform1f(glow_u_glowEffectRadius_loc, glowRadiusPixels);
    glUniform1f(glow_u_glowEffectIntensity_loc, glowEffectIntensityFactor * lightIntensity); 

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Ustawienie blendingu dla poświaty
    
    glBindVertexArray(glowVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6); // Rysowanie kwadratu (2 trójkąty)
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    shaderManager.useShaderProgram(0);
}
