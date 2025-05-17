#include "ShaderManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ShaderManager::ShaderManager() {}

ShaderManager::~ShaderManager() {
    for (auto const& [name, programID] : shaderPrograms) {
        if (programID != 0) {
            glDeleteProgram(programID);
        }
    }
    shaderPrograms.clear();
}

std::string ShaderManager::loadShaderSourceFromFile(const char* filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "ERROR::SHADER_MANAGER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << std::endl;
        return "";
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

GLuint ShaderManager::compileShader(GLenum type, const char* source, const std::string& shaderNameForLogging) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength + 1);
        glGetShaderInfoLog(shader, logLength, NULL, log.data());
        std::string shaderTypeStr = (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
        if (!shaderNameForLogging.empty()) {
             shaderTypeStr += " (" + shaderNameForLogging + ")";
        }
        std::cerr << "ERROR::SHADER_MANAGER::SHADER_COMPILATION_ERROR of type: "
                  << shaderTypeStr << "\n" << log.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool ShaderManager::linkProgram(GLuint programID, GLuint vertexShaderID, GLuint fragmentShaderID) {
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength + 1); 
        glGetProgramInfoLog(programID, logLength, NULL, log.data());
        std::cerr << "ERROR::SHADER_MANAGER::PROGRAM_LINKING_ERROR\n" << log.data() << std::endl;
        return false;
    }
    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    return true;
}

GLuint ShaderManager::loadShaderProgram(const std::string& name, const char* vertexPath, const char* fragmentPath) {
    if (shaderPrograms.count(name)) {
        return shaderPrograms[name];
    }

    std::string vertexCode = loadShaderSourceFromFile(vertexPath);
    std::string fragmentCode = loadShaderSourceFromFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty()) {
        return 0;
    }

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str(), name + "_VS");
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str(), name + "_FS");

    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return 0;
    }

    GLuint programID = glCreateProgram();
    if (!linkProgram(programID, vertexShader, fragmentShader)) {
        glDeleteProgram(programID); 
        return 0;
    }

    shaderPrograms[name] = programID;
    std::cout << "INFO::SHADER_MANAGER::Loaded shader program '" << name << "' with ID: " << programID << std::endl;
    return programID;
}

GLuint ShaderManager::getShaderProgram(const std::string& name) const {
    auto it = shaderPrograms.find(name);
    if (it != shaderPrograms.end()) {
        return it->second;
    }
    return 0;
}

void ShaderManager::useShaderProgram(const std::string& name) const {
    GLuint programID = getShaderProgram(name);
    if (programID != 0) {
        glUseProgram(programID);
    } else {
        glUseProgram(0);
    }
}

void ShaderManager::useShaderProgram(GLuint programID) const {
    glUseProgram(programID);
}

GLint ShaderManager::getUniformLocation(GLuint programID, const char* uniformName) {
    if (programID == 0) return -1;
    GLint location = glGetUniformLocation(programID, uniformName);
    return location;
}

GLint ShaderManager::getUniformLocation(const std::string& programName, const char* uniformName) {
    GLuint programID = getShaderProgram(programName);
    return getUniformLocation(programID, uniformName);
}