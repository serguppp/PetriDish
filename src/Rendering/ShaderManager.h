#pragma once

#include <GL/glew.h>
#include <string>
#include <map>
#include <vector> 

class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();

    // Wczytuje shadery z plików, kompiluje, linkuje i przechowuje pod daną nazwą.
    // Zwraca ID programu shaderowego lub 0 w przypadku błędu.
    GLuint loadShaderProgram(const std::string& name, const char* vertexPath, const char* fragmentPath);

    // Pobiera ID zapisanego programu shaderowego. Zwraca 0 jeśli nie znaleziono.
    GLuint getShaderProgram(const std::string& name) const;

    // Aktywuje program shaderowy o danej nazwie.
    void useShaderProgram(const std::string& name) const;
    // Aktywuje program shaderowy o danym ID.
    void useShaderProgram(GLuint programID) const;

    // Pobiera lokalizację uniformu 
    GLint getUniformLocation(GLuint programID, const char* uniformName);
    GLint getUniformLocation(const std::string& programName, const char* uniformName);

private:
    std::string loadShaderSourceFromFile(const char* filePath);
    GLuint compileShader(GLenum type, const char* source, const std::string& shaderNameForLogging = "");
    bool linkProgram(GLuint programID, GLuint vertexShaderID, GLuint fragmentShaderID);

    std::map<std::string, GLuint> shaderPrograms;
};