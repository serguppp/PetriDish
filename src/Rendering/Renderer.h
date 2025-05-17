#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include <memory>
#include <iostream> 
#include <map> 

#include "Simulation/IBacteria.h" 
#include "Simulation/AntibioticEffect.h"
#include "ShaderManager.h"
#include "Simulation/BacteriaStatsProvider.h" 
#include "Simulation/BacteriaStats.h" 
class Renderer {
private:
    bool initOpenGL(int width, int height); 
    void setupInitialProcedures();

    GLFWwindow* window;
    ShaderManager shaderManager;
    int windowWidth;
    int windowHeight;
    bool successfullyInitialized; 

    std::vector<AntibioticEffect> activeAntibiotics; 

    // Lokalizacje uniformów dla shadera bakterii
    GLint bacteria_u_mvp_loc;
    GLint bacteria_u_worldPosition_loc;
    GLint bacteria_u_scale_loc;
    GLint bacteria_u_bacteriaType_loc;
    GLint bacteria_u_health_loc;
    GLint bacteria_u_time_loc;

    //  Geometria Bakterii (VAO/VBO) 
    std::map<BacteriaType, GLuint> bacteriaVAOs;         
    std::map<BacteriaType, GLuint> bacteriaVBOs_pos;  
    std::map<BacteriaType, int> bacteriaVertexCounts; 
public:
    Renderer(int width, int height);
    ~Renderer();

    GLFWwindow* getWindow() const;
    bool isInitialized() const; 

    void beginFrame();
    void endFrame();

    void renderBacteria(IBacteria& bacteria, float zoomLevel);
    void renderColony(const std::vector<std::unique_ptr<IBacteria>>& allBacteria, float zoomLevel);

    void addAntibioticEffect(const glm::vec2& worldPos, float strength, float radius, float lifetime = 2.0f);
    void updateAntibioticEffects(float deltaTime);
    void renderAntibioticEffects(float zoomLevel);

    void setupBacteriaGeometry();
    void initBacteriaShader();
};