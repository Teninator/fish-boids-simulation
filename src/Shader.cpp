#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * readFile
 * Utility function to handle the I/O for shader source files.
 * Uses failbit exceptions to catch missing or unreadable files immediately.
 */
std::string Shader::readFile(const char* path) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_READ: " << path << std::endl;
        return "";
    }
}

/**
 * Standard Constructor (Vertex + Fragment)
 * Used for basic debugging or simpler objects that don't need
 * hardware tessellation.
 */
Shader::Shader(const char* vPath, const char* fPath) {
    std::string vCode = readFile(vPath);
    std::string fCode = readFile(fPath);
    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Once linked, the individual shader objects are no longer needed
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

/**
 * Advanced Constructor (Tessellation Support)
 * Extends the pipeline with Tessellation Control (TCS) and Evaluation (TES)
 * stages. This is the primary constructor used for the fish and shark models.
 */
Shader::Shader(const char* vPath, const char* fPath, const char* tcsPath, const char* tesPath) {
    std::string vCode = readFile(vPath);
    std::string fCode = readFile(fPath);
    std::string tcsCode = readFile(tcsPath);
    std::string tesCode = readFile(tesPath);

    const char* vS = vCode.c_str();
    const char* fS = fCode.c_str();
    const char* tcsS = tcsCode.c_str();
    const char* tesS = tesCode.c_str();

    unsigned int v, f, tc, te;

    // Compile sequence for all four stages
    v = glCreateShader(GL_VERTEX_SHADER); glShaderSource(v, 1, &vS, NULL); glCompileShader(v); checkCompileErrors(v, "VERTEX");

    tc = glCreateShader(GL_TESS_CONTROL_SHADER); glShaderSource(tc, 1, &tcsS, NULL); glCompileShader(tc); checkCompileErrors(tc, "TCS");

    te = glCreateShader(GL_TESS_EVALUATION_SHADER); glShaderSource(te, 1, &tesS, NULL); glCompileShader(te); checkCompileErrors(te, "TES");

    f = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(f, 1, &fS, NULL); glCompileShader(f); checkCompileErrors(f, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, v);
    glAttachShader(ID, tc);
    glAttachShader(ID, te);
    glAttachShader(ID, f);

    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Cleaning up intermediate objects
    glDeleteShader(v); glDeleteShader(tc); glDeleteShader(te); glDeleteShader(f);
}

void Shader::use() {
    glUseProgram(ID);
}

// Wrapper functions for updating Uniforms: 
// These act as the interface between the CPU-side simulation and GPU-side rendering.
void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

/**
 * checkCompileErrors
 * Debugging helper that queries the OpenGL driver for compilation/linking status.
 * Vital for catching syntax errors in GLSL code without crashing the executable.
 */
void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    int success; char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_ERROR: " << type << "\n" << infoLog << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "LINK_ERROR: " << type << "\n" << infoLog << std::endl;
        }
    }
}