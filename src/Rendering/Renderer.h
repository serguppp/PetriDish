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
#include "TextureLoader.h"

class Renderer {
private:
    bool initOpenGL(int width, int height); 
    void setupInitialProcedures();

    GLFWwindow* window;
    ShaderManager shaderManager;
    ModelLoader modelLoader;

    int windowWidth;
    int windowHeight;
    bool successfullyInitialized; 

    std::vector<AntibioticEffect> activeAntibiotics; 

    // ID programów shaderowych
    GLuint bacteriaShaderProgramID;
    GLuint antibioticShaderProgramID;

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

    // Lokalizacje uniformów dla shadera antybiotyków
    GLint antibiotic_u_modelMatrix_loc;
    GLint antibiotic_u_viewProjectionMatrix_loc;
    GLint antibiotic_u_effectColor_loc;

    // Geometria Bakterii (VAO/VBO) 
    std::map<BacteriaType, GLuint> bacteriaVAOs;         
    std::map<BacteriaType, GLuint> bacteriaVBOs_vertexLocalPosition; 
    std::map<BacteriaType, int> bacteriaVertexCounts; 

    // Geometria dla punktów (widok makro) i antybiotyków
    GLuint antibioticCircleVAO, antibioticCircleVBO_vertexPosition; 
    int antibioticCircleVertexCount;

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
    GLint petri_u_textureSampler_loc; 

    // Geometria dla poszczególnych części szalki
    GLuint dishBaseVAO, dishBaseVBO;
    size_t dishBaseVertexCount;

    GLuint dishLidVAO, dishLidVBO;
    size_t dishLidVertexCount;

    GLuint agarVAO, agarVBO;
    size_t agarVertexCount;
    GLuint agarTextureID;

    // Właściwości światła
    glm::vec3 lightPosWorld;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
    float lightRange; 

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

    void initBacteriaShader();
    void setupBacteriaGeometry();

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

    // *******************

    // ******************
    // *** Szalka i agar *** /
    void initPetriDishShader();
    void setupPetriDishGeometry(); 
    void renderPetriDish(const glm::mat4& viewProjectionMatrix, const glm::mat4& viewMatrix);

    void setupMeshGeometry(const char* modelPath, GLuint& vao, GLuint& vbo, size_t& vertexCount);

};
