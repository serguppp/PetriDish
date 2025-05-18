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

    // Lokalizacje uniformów dla oświetlenia
    GLint bacteria_u_lightPosition_world_loc;
    GLint bacteria_u_lightColor_loc;
    GLint bacteria_u_ambientColor_loc;
    GLint bacteria_u_viewPosition_world_loc; 
    GLint bacteria_u_lightRange_loc;

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

    void renderBacteria(IBacteria& bacteria, float zoomLevel, const glm::vec2& viewOffset);
    void renderColony(const std::vector<std::unique_ptr<IBacteria>>& allBacteria, float zoomLevel, const glm::vec2& viewOffset);

    void addAntibioticEffect(const glm::vec2& worldPos, float strength, float radius, float lifetime = 2.0f);
    void updateAntibioticEffects(float deltaTime);
    void renderAntibioticEffects(float zoomLevel);

    void setupBacteriaGeometry();
    void initBacteriaShader();

    void setLightPosition(const glm::vec3& pos) { lightPosWorld = pos; }
    void setLightColor(const glm::vec3& color) { lightColor = color; }
    void setAmbientColor(const glm::vec3& color) { ambientColor = color; }
    void setLightRange(float range) { lightRange = range; }

    void setupGlowGeometry();
    void initGlowShader();
    void renderGlowEffect(const glm::mat4& projection, const glm::mat4& view, float currentZoomLevel); 

    void setGlowColor(const glm::vec3& color) { glowColor = color; }
    void setGlowRadius(float radius) { glowRadius = radius; }
    void setGlowIntensity(float intensity) { glowIntensityFactor = intensity; }

};