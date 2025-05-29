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
#include "ModelLoader.h"

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

    // ID programów shaderowych
    GLuint bacteriaShaderProgramID;
    GLuint pointShaderProgramID;
    GLuint antibioticShaderProgramID;
    GLuint glowShaderProgramID;

    // Lokalizacje uniformów dla shadera bakterii (widok mikro)
    GLint bacteria_u_viewProjectionMatrix_loc;
    GLint bacteria_u_instanceWorldPosition_loc;
    GLint bacteria_u_instanceScale_loc;
    GLint bacteria_u_bacteriaType_loc;     
    GLint bacteria_u_bacteriaHealth_loc;    
    GLint bacteria_u_time_loc;

    // Lokalizacje uniformów dla oświetlenia w shaderze bakterii
    GLint bacteria_u_lightPositionWorld_loc;
    GLint bacteria_u_lightColor_loc;
    GLint bacteria_u_ambientColor_loc;
    GLint bacteria_u_cameraPositionWorld_loc; 
    GLint bacteria_u_lightRange_loc;

    // Lokalizacje uniformów dla shadera punktów (widok makro)
    GLint point_u_modelMatrix_loc;
    GLint point_u_viewProjectionMatrix_loc;
    GLint point_u_renderPointSize_loc;
    GLint point_u_pointColor_loc;

    // Lokalizacje uniformów dla shadera antybiotyków
    GLint antibiotic_u_modelMatrix_loc;
    GLint antibiotic_u_viewProjectionMatrix_loc;
    GLint antibiotic_u_effectColor_loc;

    // Geometria Bakterii (VAO/VBO) 
    std::map<BacteriaType, GLuint> bacteriaVAOs;         
    std::map<BacteriaType, GLuint> bacteriaVBOs_vertexLocalPosition; 
    std::map<BacteriaType, int> bacteriaVertexCounts; 

    // Geometria dla punktów (widok makro) i antybiotyków
    GLuint pointVAO, pointVBO_vertexPosition; 
    GLuint antibioticCircleVAO, antibioticCircleVBO_vertexPosition; 
    int antibioticCircleVertexCount;

    // Lokalizacje uniformów dla shadera poświaty
    GLuint glowVAO, glowVBO_vertexPosition; 
    GLint glow_u_lightScreenPosition_loc;
    GLint glow_u_screenResolution_loc;
    GLint glow_u_glowEffectColor_loc;
    GLint glow_u_glowEffectRadius_loc;
    GLint glow_u_glowEffectIntensity_loc;

    // Lokalizacje uniformów dla szalki
    GLuint petriDishShaderProgramID;
    GLint petri_u_modelMatrix_loc;
    GLint petri_u_viewProjectionMatrix_loc;
    GLint petri_u_normalMatrix_loc;
    GLint petri_u_objectColor_loc;
    GLint petri_u_objectAlpha_loc;
    GLint petri_u_lightPosWorld_loc;  
    GLint petri_u_lightColor_loc;
    GLint petri_u_ambientColor_loc;
    GLint petri_u_cameraPositionWorld_loc;
    GLint petri_u_lightRange_loc;

    // Geometria dla poszczególnych części szalki
    GLuint dishBaseVAO, dishBaseVBO;
    size_t dishBaseVertexCount;

    GLuint dishLidVAO, dishLidVBO;
    size_t dishLidVertexCount;

    GLuint agarVAO, agarVBO;
    size_t agarVertexCount;

    // Właściwości światła
    glm::vec3 lightPosWorld;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
    float lightRange; 
    float lightIntensity; 

    // Właściwości poświaty 
    glm::vec3 glowEffectColor;
    float glowEffectRadius;   
    float glowEffectIntensityFactor; 

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
    // *** Antybiotyki ***/
    void addAntibioticEffect(const glm::vec2& worldPos, float strength, float radius, float lifetime = 2.0f);
    void updateAntibioticEffects(float deltaTime);
    void renderAntibioticEffects(const glm::mat4& viewProjectionMatrix);
    
    void initAntibioticShader();
    void setupAntibioticGeometry();

    // *******************
    // *** Oświetlenie i Poświata ***/
    void setLightPosition(const glm::vec3& pos) { lightPosWorld = pos; }
    void setLightColor(const glm::vec3& color) { lightColor = color; }
    void setAmbientColor(const glm::vec3& color) { ambientColor = color; }
    void setLightRange(float range) { lightRange = range; }

    void initGlowShader();
    void setupGlowGeometry();

    void renderGlowEffect(const glm::mat4& projection, const glm::mat4& view, float currentZoomLevel); 

    void setGlowColor(const glm::vec3& color) { glowEffectColor = color; } 
    void setGlowRadius(float radius) { glowEffectRadius = radius; }     
    void setGlowIntensity(float intensity) { glowEffectIntensityFactor = intensity; }
    // *******************

    // ******************
    // *** Szalka i agar *** /
    void initPetriDishShader();
    void setupPetriDishGeometry(); 
    void renderPetriDish(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix);

    void setupMeshGeometry(const char* modelPath, GLuint& vao, GLuint& vbo, size_t& vertexCount);
};
