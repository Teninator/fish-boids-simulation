#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

// Definitions for older GLAD headers
#ifndef GL_TESS_CONTROL_SHADER
#define GL_TESS_CONTROL_SHADER 0x8E88
#endif
#ifndef GL_TESS_EVALUATION_SHADER
#define GL_TESS_EVALUATION_SHADER 0x8E87
#endif

class Shader {
public:
    unsigned int ID;

    // Standard constructor (Vert/Frag)
    Shader(const char* vertexPath, const char* fragmentPath);

    // Tessellation constructor (Vert/Frag/TCS/TES)
    Shader(const char* vertexPath, const char* fragmentPath, const char* tcsPath, const char* tesPath);

    void use();

    void setBool(const std::string& name, bool value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type);
    std::string readFile(const char* path);
};

#endif