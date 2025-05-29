#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include <memory>
#include <iostream> 
#include <map> 
#include <cmath>

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

    // ID shaderów
    GLuint bacteriaShaderProgramID;
    GLuint pointShaderProgramID;
    GLuint antibioticShaderProgramID;
    GLuint glowShaderProgramID;

    // Lokalizacje uniformów dla shadera bakterii w widoku mikro
    GLint bacteria_u_mvp_loc;
    GLint bacteria_u_worldPosition_loc;
    GLint bacteria_u_scale_loc;
    GLint bacteria_u_bacteriaType_loc;
    GLint bacteria_u_health_loc;
    GLint bacteria_u_time_loc;

    // Lokalizacje uniformów dla oświetlenia bakterii w widoku mikro
    GLint bacteria_u_lightPosition_world_loc;
    GLint bacteria_u_lightColor_loc;
    GLint bacteria_u_ambientColor_loc;
    GLint bacteria_u_viewPosition_world_loc; 
    GLint bacteria_u_lightRange_loc;

    // Lokalizacje uniformów dla shadera punktów (macro view)
    GLint point_u_modelMatrix_loc;
    GLint point_u_viewProjectionMatrix_loc;
    GLint point_u_pointSize_loc;
    GLint point_u_color_uniform_loc;

    // Lokalizacje uniformów dla shadera antybiotyków
    GLint antibiotic_u_modelMatrix_loc;
    GLint antibiotic_u_viewProjectionMatrix_loc;
    GLint antibiotic_u_color_loc;

    //  Geometria Bakterii (VAO/VBO) 
    std::map<BacteriaType, GLuint> bacteriaVAOs;         
    std::map<BacteriaType, GLuint> bacteriaVBOs_pos;  
    std::map<BacteriaType, int> bacteriaVertexCounts; 

    // Geometria dla punktów (macro view) i antybiotyków
    GLuint pointVAO, pointVBO;
    GLuint antibioticCircleVAO, antibioticCircleVBO;
    int antibioticCircleVertexCount;

    // Lokalizacje uniformów dla poświaty
    GLuint glowVAO, glowVBO;
    GLint glow_u_mvp_loc;
    GLint glow_u_lightPos_screen_loc;
    GLint glow_u_resolution_loc;
    GLint glow_u_glowColor_loc;
    GLint glow_u_glowRadius_loc;
    GLint glow_u_glowIntensity_loc;

    // Właściwości światła
    glm::vec3 lightPosWorld;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
    float lightRange; 
    float lightIntensity;

    // Właściwości poświaty 
    glm::vec3 glowColor;
    float glowRadius;      
    float glowIntensityFactor; 

public:
    Renderer(int width, int height);
    ~Renderer();

    GLFWwindow* getWindow() const;
    bool isInitialized() const; 

    void beginFrame();
    void endFrame();

    // *** Bakterie ***
    void renderBacteria(IBacteria& bacteria, float zoomLevel, const glm::mat4& viewProjectionMatrix);
    void renderColony(const std::vector<std::unique_ptr<IBacteria>>& allBacteria, float zoomLevel, const glm::mat4& viewProjectionMatrix);
    
    void initPointShader();
    void setupPointGeometry();

    void setupBacteriaGeometry();
    void initBacteriaShader();

    // *******************
    // *** Antybiotyki***/
    void addAntibioticEffect(const glm::vec2& worldPos, float strength, float radius, float lifetime = 2.0f);
    void updateAntibioticEffects(float deltaTime);
    void renderAntibioticEffects(const glm::mat4& viewProjectionMatrix);
    
    void initAntibioticShader();
    void setupAntibioticGeometry();

    // *******************
    // *** Oświetlenie ***/
    void setLightPosition(const glm::vec3& pos) { lightPosWorld = pos; }
    void setLightColor(const glm::vec3& color) { lightColor = color; }
    void setAmbientColor(const glm::vec3& color) { ambientColor = color; }
    void setLightRange(float range) { lightRange = range; }

    void initGlowShader();
    void setupGlowGeometry();

    void renderGlowEffect(const glm::mat4& projection, const glm::mat4& view, float currentZoomLevel); 

    void setGlowColor(const glm::vec3& color) { glowColor = color; }
    void setGlowRadius(float radius) { glowRadius = radius; }
    void setGlowIntensity(float intensity) { glowIntensityFactor = intensity; }
    // *******************

};