#include "Renderer.h"

const float MICROSCOPIC_VIEW_THRESHOLD = 1.5f;
const float BACTERIA_MODEL_SCALE_FACTOR = 0.5f;

Renderer::Renderer(int width, int height)
    : window(nullptr), windowWidth(width), windowHeight(height), successfullyInitialized(false),
      lightPosWorld(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f, 10.0f), // domyślna pozycja światła
      lightColor(1.0f, 1.0f, 1.0f),         // domyślny kolor światła 
      ambientColor(0.2f, 0.2f, 0.2f)        // domyślny kolor światła otoczenia
{
    successfullyInitialized = initOpenGL(width, height);
    if (successfullyInitialized) {
        initBacteriaShader();
        setupBacteriaGeometry();
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

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW in Renderer: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        window = nullptr;
        return false;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

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
    glEnable(GL_DEPTH_TEST); 
}

GLFWwindow* Renderer::getWindow() const {
    return window;
}

void Renderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    if (window) 
        glfwSwapBuffers(window);
    
}

void Renderer::initBacteriaShader() {
    GLuint programID = shaderManager.loadShaderProgram("bacteriaShader", "shaders/bacteria.vert", "shaders/bacteria.frag");

    if (programID == 0) {
        std::cerr << "Renderer: Failed to load bacteria shader program! Fallback rendering might be used." << std::endl;
        return;
    }

    bacteria_u_mvp_loc = shaderManager.getUniformLocation(programID, "u_mvp");
    bacteria_u_worldPosition_loc = shaderManager.getUniformLocation(programID, "u_worldPosition");
    bacteria_u_scale_loc = shaderManager.getUniformLocation(programID, "u_scale");
    bacteria_u_bacteriaType_loc = shaderManager.getUniformLocation(programID, "u_bacteriaType");
    bacteria_u_health_loc = shaderManager.getUniformLocation(programID, "u_health");
    bacteria_u_time_loc = shaderManager.getUniformLocation(programID, "u_time");

    bacteria_u_lightPosition_world_loc = shaderManager.getUniformLocation(programID, "u_lightPosition_world");
    bacteria_u_lightColor_loc = shaderManager.getUniformLocation(programID, "u_lightColor");
    bacteria_u_ambientColor_loc = shaderManager.getUniformLocation(programID, "u_ambientColor");

    if (bacteria_u_mvp_loc == -1) std::cerr << "Renderer: Uniform u_mvp not found in bacteriaShader" << std::endl;
    if (bacteria_u_worldPosition_loc == -1) std::cerr << "Renderer: Uniform u_worldPosition not found" << std::endl;
    if (bacteria_u_scale_loc == -1) std::cerr << "Renderer: Uniform u_scale not found" << std::endl;
    if (bacteria_u_bacteriaType_loc == -1) std::cerr << "Renderer: Uniform u_bacteriaType not found" << std::endl;
    if (bacteria_u_health_loc == -1) std::cerr << "Renderer: Uniform u_health not found" << std::endl;
    if (bacteria_u_time_loc == -1) std::cerr << "Renderer: Uniform u_time not found" << std::endl;

    if (bacteria_u_lightPosition_world_loc == -1) std::cerr << "Renderer: Uniform u_lightPosition_world not found" << std::endl;
    if (bacteria_u_lightColor_loc == -1) std::cerr << "Renderer: Uniform u_lightColor not found" << std::endl;
    if (bacteria_u_ambientColor_loc == -1) std::cerr << "Renderer: Uniform u_ambientColor not found" << std::endl;
}

void Renderer::setupBacteriaGeometry() {
    GLuint programID = shaderManager.getShaderProgram("bacteriaShader");
    if (programID == 0) {
        std::cerr << "Renderer: Cannot setup bacteria geometry, shader program not loaded." << std::endl;
        return;
    }

    for (int i = 0; i <= static_cast<int>(BacteriaType::Bacillus); ++i) { 
        BacteriaType type = static_cast<BacteriaType>(i);
        // Uzyskaj statystyki) dla danego typu bakterii
        BacteriaStats stats = getStatsForType(type); 

        if (stats.circuit.empty()) {
            std::cout << "Renderer: Circuit for bacteria type " << i << " is empty." << std::endl;
            continue;
        }

        std::vector<glm::vec2> vertices;
        for (const auto& p : stats.circuit) {
            vertices.push_back(glm::vec2(p.first, p.second));
        }
        
        if (vertices.empty()) { 
             std::cout << "Renderer: Vertex data for bacteria type " << i << " is empty after processing circuit." << std::endl;
            continue;
        }
        bacteriaVertexCounts[type] = static_cast<int>(vertices.size());


        GLuint vao_id, vbo_pos_id;
        glGenVertexArrays(1, &vao_id);
        glGenBuffers(1, &vbo_pos_id);

        glBindVertexArray(vao_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_id);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

        GLint posAttribLoc = glGetAttribLocation(programID, "a_position");
        if (posAttribLoc != -1) {
            glVertexAttribPointer(posAttribLoc, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
            glEnableVertexAttribArray(posAttribLoc);
        } else {
             std::cerr << "Renderer: a_position attribute not found in bacteriaShader for geometry setup." << std::endl;
        }

        bacteriaVAOs[type] = vao_id;
        bacteriaVBOs_pos[type] = vbo_pos_id;

        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glBindVertexArray(0);            
    }
     std::cout << "Renderer: Bacteria geometry setup complete." << std::endl;
}

void Renderer::renderBacteria(IBacteria& bacteria, float zoomLevel, const glm::vec2& viewOffset) {
    GLuint currentProgram = shaderManager.getShaderProgram("bacteriaShader");

    if (!bacteria.isAlive()) return;

    glm::vec4 posVec4 = bacteria.getPos();
    BacteriaType type = bacteria.getBacteriaType();

    if (zoomLevel < MICROSCOPIC_VIEW_THRESHOLD) {
        // Widok makro: kolonia bakterii jako punkty - bez shaderów, legacy OpenGL
        shaderManager.useShaderProgram(0);
        glDisable(GL_TEXTURE_2D);   

        switch (type) {
            case BacteriaType::Cocci: glColor3f(0.9f, 0.4f, 0.4f); break;
            case BacteriaType::Diplococcus: glColor3f(0.4f, 0.9f, 0.4f); break;
            case BacteriaType::Staphylococci: glColor3f(0.4f, 0.4f, 0.9f); break;
            case BacteriaType::Bacillus: glColor3f(0.8f, 0.6f, 0.2f); break; 
            default: glColor3f(0.7f, 0.7f, 0.7f); break;
        }

        float pointSize = 3.0f;
        glPointSize(pointSize);
        glBegin(GL_POINTS);
        glVertex2f(posVec4.x, posVec4.y);
        glEnd();

    } else {
        // Widok mikro z shaderami
        if (currentProgram == 0) { // jeśli shader niezaładowany
            shaderManager.useShaderProgram(0); 
            glPushMatrix();
            glTranslatef(posVec4.x, posVec4.y, 0.0f);
            glScalef(BACTERIA_MODEL_SCALE_FACTOR, BACTERIA_MODEL_SCALE_FACTOR, 1.0f);
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
                GLfloat baseColor[4]; glGetFloatv(GL_CURRENT_COLOR, baseColor);
                glColor4f(baseColor[0] * health, baseColor[1] * health, baseColor[2] * health, (health > 0.1f ? 1.0f : health * 2.0f) );
                glBegin(GL_POLYGON);
                for (const auto& vertexOffset : circuit) {
                    glVertex2f(static_cast<float>(vertexOffset.first), static_cast<float>(vertexOffset.second));
                }
                glEnd();
            }
            glPopMatrix();
            return; 
        }

        shaderManager.useShaderProgram(currentProgram);

        // --- Macierze ---
        // W main.cpp jest już ustawione glOrtho i glLoadIdentity dla MODELVIEW.
        // Shader vertex (bacteria.vert) oczekuje:
        // u_mvp: (projection * view) - globalne transformacje kamery
        // u_worldPosition: pozycja środka bakterii
        // u_scale: skala bakterii
        // Nalezy pobrac aktualne macierze widoku i projekcji z main.cpp aby je przenieść do shadera 

        GLfloat projMatrixGL[16];
        GLfloat modelViewMatrixGL[16];

        glGetFloatv(GL_PROJECTION_MATRIX, projMatrixGL);
        glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrixGL); 

        glm::mat4 projection = glm::make_mat4(projMatrixGL);
        glm::mat4 view = glm::make_mat4(modelViewMatrixGL);
        glm::mat4 mvp_for_shader = projection * view; 

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

        // Obliczanie pozycji kamery w świecie 
        float view_width_world = static_cast<float>(windowWidth) / zoomLevel;
        float view_height_world = static_cast<float>(windowHeight) / zoomLevel;
        glm::vec3 viewPosWorld(viewOffset.x + view_width_world / 2.0f,
                               viewOffset.y + view_height_world / 2.0f,
                               100.0f); 
        glUniform3fv(bacteria_u_viewPosition_world_loc, 1, glm::value_ptr(viewPosWorld));

        auto vao_it = bacteriaVAOs.find(type);
        if (vao_it != bacteriaVAOs.end() && bacteriaVertexCounts.count(type) && bacteriaVertexCounts.at(type) > 0) {
            glBindVertexArray(vao_it->second);
            glDrawArrays(GL_TRIANGLE_FAN, 0, bacteriaVertexCounts.at(type));
            glBindVertexArray(0);
        }
        shaderManager.useShaderProgram(0);
    }
}


void Renderer::renderColony(const std::vector<std::unique_ptr<IBacteria>>& allBacteria, float zoomLevel, const glm::vec2& viewOffset) {
    for (const auto& bacteriaPtr : allBacteria) {
        if (bacteriaPtr) {
            renderBacteria(*bacteriaPtr, zoomLevel, viewOffset); 
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