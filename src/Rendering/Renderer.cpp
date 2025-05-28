#include "Renderer.h"

const float MICROSCOPIC_VIEW_THRESHOLD = 1.5f;
const float BACTERIA_MODEL_SCALE_FACTOR = 0.5f;

Renderer::Renderer(int width, int height)
    : window(nullptr), windowWidth(width), windowHeight(height), successfullyInitialized(false),
      lightPosWorld(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f, 10.0f), // domyślna pozycja światła
      lightColor(1.5f, 1.5f, 1.5f),         // domyślny kolor światła 
      ambientColor(0.5f, 0.5f, 0.5f), // domyślny kolor światła otoczenia
      lightRange(100.0f), // odleglosc swiatla 
      lightIntensity(1.0f), // intensywność światła
      glowColor(1.0f, 0.9f, 0.7f), // Kolor poświaty
      glowRadius(100.0f),         // Promień poświaty 
      glowIntensityFactor(0.75f)   // Intensywność poświaty
{
    successfullyInitialized = initOpenGL(width, height);
    if (successfullyInitialized) {
        // Inicjalizacja shaderów po inicjalizacji środowiska openGL
        initBacteriaShader();
        setupBacteriaGeometry();
        initGlowShader();     
        setupGlowGeometry();  
    }
}

Renderer::~Renderer() {
    for (auto const& [type, vaoID] : bacteriaVAOs) {
        if (vaoID != 0) glDeleteVertexArrays(1, &vaoID);
    }
    bacteriaVAOs.clear();

    for (auto const& [type, vboID] : bacteriaVBOs_pos) {
        if (vboID != 0) glDeleteBuffers(1, &vboID);
    }
    bacteriaVBOs_pos.clear();
    
    bacteriaVertexCounts.clear(); 

    if (!activeAntibiotics.empty()) {
         activeAntibiotics.clear();
    }

    if (window) {
        glfwDestroyWindow(window);
    }
}

bool Renderer::initOpenGL(int width, int height) {
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //OpenGL 3.0+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    //Tworzenie okna
    window = glfwCreateWindow(width, height, "PetriDish Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window in Renderer" << std::endl;
        const char* description;
        glfwGetError(&description);
        if (description) {
            std::cerr << "GLFW Error: " << description << std::endl;
        }
        return false;
    }
    glfwMakeContextCurrent(window);

    //Inicjalizacja glew
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW in Renderer: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        window = nullptr;
        return false;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    //Uruchamianie podstawowych ustawień renderowania naszego świata
    setupInitialProcedures();
    // v-sync
    glfwSwapInterval(1);
    return true;
}

bool Renderer::isInitialized() const {
    return successfullyInitialized;
}

// Podstawowe ustawienia renderowania naszego swiata
void Renderer::setupInitialProcedures() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // kolor tla
    glEnable(GL_BLEND); // wlaczanie blendingu czyli mieszania kolorów
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ustawienia blendingu kolorów
    glEnable(GL_POINT_SMOOTH); // wygladzanie punktów GL_Points
    glEnable(GL_DEPTH_TEST);  // testowanie głębi, z-bufor
}

GLFWwindow* Renderer::getWindow() const {
    return window;
}

// Co klatkę resetujemy bufor koloru i głębbi
void Renderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Co klatkę zamieniamy bufor z narysowna scena - tylni z przednim 
void Renderer::endFrame() {
    if (window) 
        glfwSwapBuffers(window);
    
}

// Inicjalizacja shadera bakterii
void Renderer::initBacteriaShader() {
    GLuint programID = shaderManager.loadShaderProgram("bacteriaShader", "shaders/bacteria.vert", "shaders/bacteria.frag");

    if (programID == 0) {
        std::cerr << "Renderer: Failed to load bacteria shader program! Fallback rendering might be used." << std::endl;
        return;
    }

    // pobieranie lokalizacji uniformów z shadera
    bacteria_u_mvp_loc = shaderManager.getUniformLocation(programID, "u_mvp"); //macierz mvp
    bacteria_u_worldPosition_loc = shaderManager.getUniformLocation(programID, "u_worldPosition"); //pozycja bakterii
    bacteria_u_scale_loc = shaderManager.getUniformLocation(programID, "u_scale"); //skala obiektu bakterii
    bacteria_u_bacteriaType_loc = shaderManager.getUniformLocation(programID, "u_bacteriaType"); //typ bakterii
    bacteria_u_health_loc = shaderManager.getUniformLocation(programID, "u_health"); // hp bakterii
    bacteria_u_time_loc = shaderManager.getUniformLocation(programID, "u_time"); // zmienna czasowa do animacji

    bacteria_u_lightPosition_world_loc = shaderManager.getUniformLocation(programID, "u_lightPosition_world"); // pozycja swiatla w przestrzeni swiata
    bacteria_u_lightColor_loc = shaderManager.getUniformLocation(programID, "u_lightColor"); // emitowany kolor przez zrodlo swiatla
    bacteria_u_ambientColor_loc = shaderManager.getUniformLocation(programID, "u_ambientColor"); // kolor siwatla otoczenia 
    bacteria_u_viewPosition_world_loc = shaderManager.getUniformLocation(programID, "u_viewPosition_world");  //pozycja kamery
    bacteria_u_lightRange_loc = shaderManager.getUniformLocation(programID, "u_lightRange");  // zasieg swiatla

    if (bacteria_u_mvp_loc == -1) std::cerr << "Renderer: Uniform u_mvp not found in bacteriaShader" << std::endl;
    if (bacteria_u_worldPosition_loc == -1) std::cerr << "Renderer: Uniform u_worldPosition not found" << std::endl;
    if (bacteria_u_scale_loc == -1) std::cerr << "Renderer: Uniform u_scale not found" << std::endl;
    if (bacteria_u_bacteriaType_loc == -1) std::cerr << "Renderer: Uniform u_bacteriaType not found" << std::endl;
    if (bacteria_u_health_loc == -1) std::cerr << "Renderer: Uniform u_health not found" << std::endl;
    if (bacteria_u_time_loc == -1) std::cerr << "Renderer: Uniform u_time not found" << std::endl;

    if (bacteria_u_lightPosition_world_loc == -1) std::cerr << "Renderer: Uniform u_lightPosition_world not found" << std::endl;
    if (bacteria_u_lightColor_loc == -1) std::cerr << "Renderer: Uniform u_lightColor not found" << std::endl;
    if (bacteria_u_ambientColor_loc == -1) std::cerr << "Renderer: Uniform u_ambientColor not found" << std::endl;
    if (bacteria_u_viewPosition_world_loc == -1) std::cerr << "Renderer: Uniform u_viewPosition_world not found" << std::endl; 
    if (bacteria_u_lightRange_loc == -1) std::cerr << "Renderer: Uniform u_lightRange not found" << std::endl; 
}

// Przekazywanie informacji o geometrii danych bakterii do renderera na podstawie circuit w BacteriaStats
void Renderer::setupBacteriaGeometry() {
    GLuint programID = shaderManager.getShaderProgram("bacteriaShader");
    if (programID == 0) {
        std::cerr << "Renderer: Cannot setup bacteria geometry, shader program not loaded." << std::endl;
        return;
    }

    for (int i = 0; i <= static_cast<int>(BacteriaType::Bacillus); ++i) { 
        BacteriaType type = static_cast<BacteriaType>(i);
        // Uzyskaj statystyki dla danego typu bakterii
        BacteriaStats stats = getStatsForType(type); 

        if (stats.circuit.empty()) {
            std::cout << "Renderer: Circuit for bacteria type " << i << " is empty." << std::endl;
            continue;
        }

        // Punkty w obwodzie typu bakterii przekładamy na wierzchołki
        std::vector<glm::vec2> vertices;
        for (const auto& p : stats.circuit) {
            vertices.push_back(glm::vec2(p.first, p.second));
        }
        
        if (vertices.empty()) { 
             std::cout << "Renderer: Vertex data for bacteria type " << i << " is empty after processing circuit." << std::endl;
            continue;
        }
        bacteriaVertexCounts[type] = static_cast<int>(vertices.size());

        // Generowanie vertex array object i vertex buffer object
        GLuint vao_id, vbo_pos_id;
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(1, &vbo_pos_id);

        // aktywacja vao i vbo; wyslanie danych wierzchołków do gpu
        glBindVertexArray(vao_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_id);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

        // konfiguracja atrybutów wierzchołków przez shader
        GLint posAttribLoc = glGetAttribLocation(programID, "a_position");
        if (posAttribLoc != -1) {
            glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
            glEnableVertexAttribArray(posAttribLoc);
        } else {
             std::cerr << "Renderer: a_position attribute not found in bacteriaShader for geometry setup." << std::endl;
        }

        // Zapis VAO i VBO dla danego typu bakterii w shaderze
        bacteriaVAOs[type] = vao_id;
        bacteriaVBOs_pos[type] = vbo_pos_id;

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0);            
    }
     std::cout << "Renderer: Bacteria geometry setup complete." << std::endl;
}

void Renderer::renderBacteria(IBacteria& bacteria, float zoomLevel, const glm::vec2& viewOffset) {
    GLuint currentProgram = shaderManager.getShaderProgram("bacteriaShader");

    // Jeżeli jakimś cudem w systemie istnieje bakteria, która nie żyje, to nie renderujemy jej
    if (!bacteria.isAlive()) return;

    // Pobieranie pozycji i typu bakterii
    glm::vec4 posVec4 = bacteria.getPos();
    BacteriaType type = bacteria.getBacteriaType();

    // Widok bakterii zależny  od zoomu
    if (zoomLevel < MICROSCOPIC_VIEW_THRESHOLD) {
        // Widok makro: kolonia bakterii jako punkty - bez shaderów, legacy OpenGL
        shaderManager.useShaderProgram(0);
        glDisable(GL_TEXTURE_2D);   

        // Kolory różnych typów bakterii
        switch (type) {
            case BacteriaType::Cocci: glColor3f(0.9f, 0.4f, 0.4f); break;
            case BacteriaType::Diplococcus: glColor3f(0.4f, 0.9f, 0.4f); break;
            case BacteriaType::Staphylococci: glColor3f(0.4f, 0.4f, 0.9f); break;
            case BacteriaType::Bacillus: glColor3f(0.8f, 0.6f, 0.2f); break; 
            default: glColor3f(0.7f, 0.7f, 0.7f); break;
        }

        // Rysowanie  bakterii jako punkty
        float pointSize = 3.0f;
        glPointSize(pointSize);
        glBegin(GL_POINTS);
        glVertex2f(posVec4.x, posVec4.y);
        glEnd();

    } else {
        // Widok mikro z shaderami
        if (currentProgram == 0) { // jeśli shader niezaładowany; awaryjne rysowanie jako wielokąty zależne od circuit
            shaderManager.useShaderProgram(0);  
            glPushMatrix(); // zapisanie macierzy mvp
            glTranslatef(posVec4.x, posVec4.y, 0.0f); //przesuwanie 
            glScalef(BACTERIA_MODEL_SCALE_FACTOR, BACTERIA_MODEL_SCALE_FACTOR, 1.0f); //skalowanie
            const auto& circuit = bacteria.getCircuit();
            if (!circuit.empty()) {
                float health = bacteria.getHealth();
                switch (type) {
                    case BacteriaType::Cocci: glColor3f(0.9f, 0.4f, 0.4f); break;
                    case BacteriaType::Diplococcus: glColor3f(0.4f, 0.9f, 0.4f); break;
                    case BacteriaType::Staphylococci: glColor3f(0.4f, 0.4f, 0.9f); break;
                    case BacteriaType::Bacillus: glColor3f(0.8f, 0.6f, 0.2f); break;
                    default: glColor3f(0.7f, 0.7f, 0.7f); break;
                }
                //Modyfikacja koloru bakterii na podstawie zdrowia
                GLfloat baseColor[4]; glGetFloatv(GL_CURRENT_COLOR, baseColor);
                glColor4f(baseColor[0] * health, baseColor[1] * health, baseColor[2] * health, (health > 0.1f ? 1.0f : health * 2.0f) );
                glBegin(GL_POLYGON);
                for (const auto& vertexOffset : circuit) {
                    glVertex2f(static_cast<float>(vertexOffset.first), static_cast<float>(vertexOffset.second));
                }
                glEnd();
            }
            //przywracanie macierzy mvp
            glPopMatrix();
            return; 
        }

        shaderManager.useShaderProgram(currentProgram);

        // --- Macierze ---
        // W main.cpp jest już ustawione glOrtho i glLoadIdentity dla MODELVIEW.
        // Shader vertex (bacteria.vert) oczekuje:
        // u_mvp: (projection * view) - macierze bakterii
        // u_worldPosition: pozycja środka swiata
        // u_scale: skala bakterii
        // Nalezy pobrac aktualne macierze widoku i projekcji z main.cpp aby je przenieść do shadera 

        GLfloat projMatrixGL[16];
        GLfloat modelViewMatrixGL[16];

        glGetFloatv(GL_PROJECTION_MATRIX, projMatrixGL); // aktualna macierz projekcji
        glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrixGL);  // aktualna macierz ModelView

        glm::mat4 projection = glm::make_mat4(projMatrixGL);
        glm::mat4 view = glm::make_mat4(modelViewMatrixGL);
        glm::mat4 mvp_for_shader = projection * view; 

        //Przekazanie macierzy jako uniformy do shadera
        glUniformMatrix4fv(bacteria_u_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp_for_shader));
        glUniform2f(bacteria_u_worldPosition_loc, posVec4.x, posVec4.y);
        glUniform1f(bacteria_u_scale_loc, BACTERIA_MODEL_SCALE_FACTOR);
        glUniform1i(bacteria_u_bacteriaType_loc, static_cast<int>(type));
        glUniform1f(bacteria_u_health_loc, bacteria.getHealth());
        glUniform1f(bacteria_u_time_loc, static_cast<float>(glfwGetTime()));

        // Pozycja światła 
        glUniform3fv(bacteria_u_lightPosition_world_loc, 1, glm::value_ptr(lightPosWorld));
        glUniform3fv(bacteria_u_lightColor_loc, 1, glm::value_ptr(lightColor));
        glUniform3fv(bacteria_u_ambientColor_loc, 1, glm::value_ptr(ambientColor));
        glUniform1f(bacteria_u_lightRange_loc, lightRange);

        // Obliczanie pozycji kamery w świecie 
        float view_width_world = static_cast<float>(windowWidth) / zoomLevel;
        float view_height_world = static_cast<float>(windowHeight) / zoomLevel;
        glm::vec3 viewPosWorld(viewOffset.x + view_width_world / 2.0f,
                               viewOffset.y + view_height_world / 2.0f,
                               100.0f); 
        glUniform3fv(bacteria_u_viewPosition_world_loc, 1, glm::value_ptr(viewPosWorld));

        // Rysowanie VAO bakterii
        auto vao_it = bacteriaVAOs.find(type);
        if (vao_it != bacteriaVAOs.end() && bacteriaVertexCounts.count(type) && bacteriaVertexCounts.at(type) > 0) {
            glBindVertexArray(vao_it->second);
            glDrawArrays(GL_TRIANGLE_FAN, 0, bacteriaVertexCounts.at(type));
            glBindVertexArray(0);
        }
        shaderManager.useShaderProgram(0);
    }
}

// Renderowanie bakterii
void Renderer::renderColony(const std::vector<std::unique_ptr<IBacteria>>& allBacteria, float zoomLevel, const glm::vec2& viewOffset) {
    for (const auto& bacteriaPtr : allBacteria) {
        if (bacteriaPtr) {
            renderBacteria(*bacteriaPtr, zoomLevel, viewOffset); 
        }
    }
}

// Dodawanie antybiotyków do listy 
void Renderer::addAntibioticEffect(const glm::vec2& worldPos, float strength, float radius, float lifetime) {
    activeAntibiotics.push_back({worldPos, strength, radius, 0.0f, lifetime});
}

// Funkcja odpowiedzialna za działanie antybiotyku na bakterie
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

// Funkcja odpowiedzialna za renderowanie poświaty antybiotyku, która kurczy się w czasie
void Renderer::renderAntibioticEffects(float zoomLevel) {
    shaderManager.useShaderProgram(0); 
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
}

void Renderer::initGlowShader() {
    GLuint programID = shaderManager.loadShaderProgram("glowShader", "shaders/glow.vert", "shaders/glow.frag");
    if (programID == 0) {
        std::cerr << "Renderer: Failed to load glow shader program!" << std::endl;
        return;
    }
    glow_u_mvp_loc = shaderManager.getUniformLocation(programID, "u_mvp");
    glow_u_lightPos_screen_loc = shaderManager.getUniformLocation(programID, "u_lightPos_screen");
    glow_u_resolution_loc = shaderManager.getUniformLocation(programID, "u_resolution");
    glow_u_glowColor_loc = shaderManager.getUniformLocation(programID, "u_glowColor");
    glow_u_glowRadius_loc = shaderManager.getUniformLocation(programID, "u_glowRadius");
    glow_u_glowIntensity_loc = shaderManager.getUniformLocation(programID, "u_glowIntensity");
}

void Renderer::setupGlowGeometry() {
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
    glGenBuffers(1, &glowVBO);
    glBindVertexArray(glowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, glowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void Renderer::renderGlowEffect(const glm::mat4& projection, const glm::mat4& view, float currentZoomLevel) {
    GLuint glowProgramID = shaderManager.getShaderProgram("glowShader");
    if (glowProgramID == 0) return;

    shaderManager.useShaderProgram(glowProgramID);

    glm::mat4 mvp = glm::mat4(1.0f); 
    glUniformMatrix4fv(glow_u_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

    glm::vec4 lightPosClip = projection * view * glm::vec4(lightPosWorld, 1.0);
    glm::vec3 lightPosNDC = glm::vec3(lightPosClip.x / lightPosClip.w, lightPosClip.y / lightPosClip.w, lightPosClip.z / lightPosClip.w);
    glm::vec2 lightPosScreen(
        (lightPosNDC.x * 0.5f + 0.5f) * windowWidth,
        (lightPosNDC.y * 0.5f + 0.5f) * windowHeight
    );

    glUniform2fv(glow_u_lightPos_screen_loc, 1, glm::value_ptr(lightPosScreen));
    glUniform2f(glow_u_resolution_loc, static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    glUniform3fv(glow_u_glowColor_loc, 1, glm::value_ptr(glowColor));

    float glowRadiusPixels = glowRadius * currentZoomLevel; 
    glUniform1f(glow_u_glowRadius_loc, glowRadiusPixels);
    glUniform1f(glow_u_glowIntensity_loc, glowIntensityFactor * lightIntensity); 

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glBindVertexArray(glowVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    shaderManager.useShaderProgram(0);
}