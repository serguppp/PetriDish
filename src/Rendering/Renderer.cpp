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
      lightPosWorld(0.0f, 0.0f, 50.0f), 
      lightColor(1.5f, 1.5f, 1.5f),         
      ambientColor(0.5f, 0.5f, 0.5f), 
      lightRange(200.0f),  
      agarTextureID(0){

    successfullyInitialized = initOpenGL(width, height);
    if (successfullyInitialized) {
        // Inicjalizacja shaderów po pomyślnym utworzeniu kontekstu OpenGL
        initBacteriaShader();
        setupBacteriaGeometry();

        initAntibioticShader();
        setupAntibioticGeometry();

        initPetriDishShader();
        setupPetriDishGeometry(); 
        
        agarTextureID = TextureLoader::loadTexture("assets/textures/Leather024_1K-JPG_Color.jpg");
        if (agarTextureID == 0) std::cerr << "Błąd załadowania tekstury szalki." << std::endl;
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

    // Czyszczenie zasobów
    if (antibioticCircleVAO != 0) glDeleteVertexArrays(1, &antibioticCircleVAO);
    if (antibioticCircleVBO_vertexPosition != 0) glDeleteBuffers(1, &antibioticCircleVBO_vertexPosition);

    if (dishBaseVAO != 0) glDeleteVertexArrays(1, &dishBaseVAO);
    if (dishBaseVBO != 0) glDeleteBuffers(1, &dishBaseVBO);
    if (dishLidVAO != 0) glDeleteVertexArrays(1, &dishLidVAO);
    if (dishLidVBO != 0) glDeleteBuffers(1, &dishLidVBO);
    if (agarVAO != 0) glDeleteVertexArrays(1, &agarVAO);
    if (agarVBO != 0) glDeleteBuffers(1, &agarVBO);

    if (!activeAntibiotics.empty()) {
         activeAntibiotics.clear();
    }

    if (agarTextureID != 0) {
        glDeleteTextures(1, &agarTextureID);
    }

    if (window) {
        glfwDestroyWindow(window);
    }
}

bool Renderer::initOpenGL(int width, int height) {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); 

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
    glDepthMask(GL_TRUE);
    glfwSwapInterval(1); // Włączenie V-Sync
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
        glfwSwapBuffers(window); // Zamiana buforów przedni z tylnym
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

        // Widok mikro: renderowanie pełnego modelu bakterii
        if (bacteriaShaderProgramID == 0 || bacteriaVAOs.find(type) == bacteriaVAOs.end()) return;

        shaderManager.useShaderProgram(bacteriaShaderProgramID);

        // Ustawianie uniformów dla shadera bakterii
        glUniformMatrix4fv(bacteria_u_viewProjectionMatrix_loc, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
        glUniform3f(bacteria_u_instanceWorldPosition_loc, posVec4.x, posVec4.y, posVec4.z); 
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

// Inicjalizacja shadera dla wzsystkich elementów szalki
void Renderer::initPetriDishShader() {
    petriDishShaderProgramID = shaderManager.loadShaderProgram("petriDishShader", "shaders/petridish.vert", "shaders/petridish.frag");
    if (petriDishShaderProgramID == 0) {
        std::cerr << "Renderer: Błąd ładowania programu shadera dla szalki!" << std::endl;
        return;
    }
    petri_u_modelMatrix_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_modelMatrix");
    petri_u_viewProjectionMatrix_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_viewProjectionMatrix");
    petri_u_normalMatrix_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_normalMatrix");
    petri_u_objectColor_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_objectColor");
    petri_u_objectAlpha_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_objectAlpha");
    
    petri_u_lightPosWorld_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_lightPosWorld");
    petri_u_lightColor_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_lightColor");
    petri_u_ambientColor_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_ambientColor");
    petri_u_cameraPositionWorld_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_cameraPositionWorld");
    petri_u_lightRange_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "u_lightRange");
    petri_u_textureSampler_loc = shaderManager.getUniformLocation(petriDishShaderProgramID, "uTextureSampler");

}

// Przekazanie geometrii  obiektów z Blendera do VAO i VBO
void Renderer::setupPetriDishGeometry() {
    setupMeshGeometry("assets/models/szalka.obj", dishBaseVAO, dishBaseVBO, dishBaseVertexCount);
    setupMeshGeometry("assets/models/przykrywka.obj", dishLidVAO, dishLidVBO, dishLidVertexCount);
    setupMeshGeometry("assets/models/agar.obj", agarVAO, agarVBO, agarVertexCount);
}

// Renderowanie szalki: przekazanie uniformów do shadera, kolor, oteksturowanie, transparentnosc
void Renderer::renderPetriDish(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix) {
    if (petriDishShaderProgramID == 0) return;

    shaderManager.useShaderProgram(petriDishShaderProgramID);

    // Uniformy dla wszystkich części szalki 
    glm::vec3 cameraPosForShader = glm::vec3(glm::inverse(viewMatrix)[3]);

    glUniform3fv(petri_u_lightPosWorld_loc, 1, glm::value_ptr(lightPosWorld));
    glUniform3fv(petri_u_lightColor_loc, 1, glm::value_ptr(lightColor));
    glUniform3fv(petri_u_ambientColor_loc, 1, glm::value_ptr(ambientColor));
    glUniform3fv(petri_u_cameraPositionWorld_loc, 1, glm::value_ptr(cameraPosForShader));
    glUniform1f(petri_u_lightRange_loc, lightRange); 
    
    glUniformMatrix4fv(petri_u_viewProjectionMatrix_loc, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

    // Renderowanie przezroczystych części szalki
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Wyłącz zapis do bufora głębi dla przezroczystych obiektów
    glEnable(GL_CULL_FACE); // Włącz odrzucanie ścian
    glCullFace(GL_BACK);    // Odrzucaj tylne ściany

    // Renderowanie podstawy szalki
    if (dishBaseVAO != 0 && dishBaseVertexCount > 0) {
        glm::mat4 modelMatrix = glm::mat4(1.0f); 
        modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f)); 
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        
        glUniformMatrix4fv(petri_u_modelMatrix_loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(petri_u_normalMatrix_loc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        
        glUniform3f(petri_u_objectColor_loc, 0.85f, 0.9f, 0.95f); // Kolor szkła
        glUniform1f(petri_u_objectAlpha_loc, 0.15);            // Alpha szkła

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0); 
        glUniform1i(petri_u_textureSampler_loc, 0);

        glBindVertexArray(dishBaseVAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(dishBaseVertexCount));
        
    }

    // Renderowanie agaru 
    if (agarVAO != 0 && agarVertexCount > 0) {
        glm::mat4 modelMatrix = glm::mat4(1.0f); 
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -1.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f)); 
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

        glUniformMatrix4fv(petri_u_modelMatrix_loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(petri_u_normalMatrix_loc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        glUniform3f(petri_u_objectColor_loc, 1.0f, 1.0f, 1.0f);
        glUniform1f(petri_u_objectAlpha_loc, 0.9f); 

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, agarTextureID);
        glUniform1i(petri_u_textureSampler_loc, 0); 

        glBindVertexArray(agarVAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(agarVertexCount));
    }

    // Renderowanie przykrywki szalki
    if (dishLidVAO != 0 && dishLidVertexCount > 0) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f)); 
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));       
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

        glUniformMatrix4fv(petri_u_modelMatrix_loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix3fv(petri_u_normalMatrix_loc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        glUniform3f(petri_u_objectColor_loc, 0.85f, 0.9f, 0.95f); // Kolor szkła
        glUniform1f(petri_u_objectAlpha_loc, 0.15f);            // Alpha szkła
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0); 
        glUniform1i(petri_u_textureSampler_loc, 0);

        glBindVertexArray(dishLidVAO);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(dishLidVertexCount));
    }

    // koniec renderowanie przezroczystych części szalki
    glDisable(GL_CULL_FACE); // Wyłączanie  odrzucania ścian
    glDepthMask(GL_TRUE);    // Przywrocenie zapisu do bufora głębi
    glDisable(GL_BLEND);   
    glBindVertexArray(0);
    shaderManager.useShaderProgram(0);
}

// Ustalanie siatki modelu
void Renderer::setupMeshGeometry(const char* modelPath, GLuint& vao, GLuint& vbo, size_t& vertexCount) {
    std::vector<Vertex> vertices;
    if (!modelLoader.loadOBJ(modelPath, vertices) || vertices.empty()) {
        std::cerr << "Renderer: Nie udało się załadować modelu lub model jest pusty: " << modelPath << std::endl;
        vao = 0; vbo = 0; vertexCount = 0;
        return;
    }
    vertexCount = vertices.size();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    std::cout << "INFO::Renderer: Załadowano model " << modelPath << " (" << vertexCount << " wierzchołków)" << std::endl;
}
