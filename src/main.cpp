#define GLM_FORCE_RADIANS
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "tiny_obj_loader.h"
#include <iostream>
#include <vector>
#include "Shader.h"
#include "Flock.h"

// Definition for Tessellation Patches if not defined by the header
#ifndef GL_PATCHES
#define GL_PATCHES 0x000E
#define GL_PATCH_VERTICES 0x8E72
#endif

typedef void (APIENTRYP PFNGLPATCHPARAMETERIPROC)(GLenum pname, GLint value);

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Particle structure for environmental visual noise
struct Bubble { glm::vec3 pos; float life; float speed; };
std::vector<Bubble> bubbles;

// Manual obstacle placement for the corals
std::vector<glm::vec3> obstacles = {
    glm::vec3(40, 0, 40),
    glm::vec3(-40, 10, -30),
    glm::vec3(0, -20, -60)
};

// Fixed high-altitude camera for the top-down perspective
glm::vec3 cameraPos = glm::vec3(0.0f, 150.0f, 0.0f);

/**
  loadMesh
  Utilizes tiny_obj_loader to parse vertex data and upload it to a VAO. Optimized for a single vertex attribute (Position) to be processed by the Tessellation shaders.
 */
int loadMesh(const char* path, unsigned int& vao) {
    tinyobj::attrib_t attr;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> mats;
    std::string warn, err;
    if (!tinyobj::LoadObj(&attr, &shapes, &mats, &warn, &err, path)) return 0;

    std::vector<float> vertices;
    for (const auto& shape : shapes) {
        for (const auto& idx : shape.mesh.indices) {
            vertices.push_back(attr.vertices[3 * idx.vertex_index + 0]);
            vertices.push_back(attr.vertices[3 * idx.vertex_index + 1]);
            vertices.push_back(attr.vertices[3 * idx.vertex_index + 2]);
        }
    }
    unsigned int vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    return (int)(vertices.size() / 3);
}

int main() {
    // Window and OpenGL Context Initialization
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CSCI 4110U: The Deep", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Setup for Hardware Tessellation
    PFNGLPATCHPARAMETERIPROC glPatchParameteri_ptr = (PFNGLPATCHPARAMETERIPROC)glfwGetProcAddress("glPatchParameteri");
    if (glPatchParameteri_ptr) glPatchParameteri_ptr(GL_PATCH_VERTICES, 3);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); // Enabled for bubble transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Loads the 4-stage shader pipeline
    Shader shader("shaders/boid.vert", "shaders/boid.frag", "shaders/boid.tcs", "shaders/boid.tes");

    unsigned int fishVAO, sharkVAO, sphereVAO;
    int fV = loadMesh("assets/models/Fish.obj", fishVAO);
    int sV = loadMesh("assets/models/Shark.obj", sharkVAO);
    int bV = loadMesh("assets/models/Sphere.obj", sphereVAO);

    Flock flock(120);
    float lastFrame = 0.0f;
    static glm::vec3 smoothSharkFront = glm::vec3(0, 0, 1);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        float dt = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (dt > 0.05f) dt = 0.05f; // Frame capping to prevent physics tunneling

        flock.update(dt, obstacles);

        glClearColor(0.01f, 0.01f, 0.06f, 1.0f); // Deep sea blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setFloat("time", currentFrame);

        // Matrix Setup: Projection and View
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
        shader.setMat4("projection", glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.f));
        shader.setMat4("view", view);

        // ---  Corals (STATIC) ---
        // Resets animation flags so the environment stays rigid
        shader.setBool("isShark", false);
        shader.setBool("isFish", false);
        glBindVertexArray(sphereVAO);
        shader.setVec3("color", glm::vec3(0.0f, 0.6f, 0.6f));
        for (auto& pos : obstacles) {
            shader.setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), pos), glm::vec3(10.0f)));
            glDrawArrays(GL_PATCHES, 0, bV);
        }

        // ---  Fish  ---
        shader.setBool("isFish", true);
        glBindVertexArray(fishVAO);
        for (auto& f : flock.fishes) {
            shader.setVec3("color", f.color);
            glm::mat4 m = glm::translate(glm::mat4(1.0f), f.position);

            if (glm::length(f.velocity) > 0.1f) {
                // Procedural bubble spawning based on movement
                if (((float)rand() / RAND_MAX) < 0.012f) {
                    bubbles.push_back({ f.position, 1.5f, 3.5f });
                }
                // Constructs the rotation matrix from the velocity vector
                glm::vec3 front = glm::normalize(f.velocity);
                glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), front));
                m *= glm::mat4(glm::vec4(right, 0), glm::vec4(glm::cross(front, right), 0), glm::vec4(front, 0), glm::vec4(0, 0, 0, 1));
            }
            shader.setMat4("model", glm::scale(m, glm::vec3(0.12f)));
            glDrawArrays(GL_PATCHES, 0, fV);
        }

        // --- Shark ---
        shader.setBool("isShark", true);
        shader.setBool("isFish", false);
        glBindVertexArray(sharkVAO);
        shader.setVec3("color", glm::vec3(0.85f, 0.1f, 0.1f));

        if (glm::length(flock.predator.velocity) > 0.1f) {
            glm::vec3 actualFront = glm::normalize(flock.predator.velocity);
            // Visual Smoothing: Blends the orientation for smoother shark banking
            smoothSharkFront = glm::normalize(glm::mix(smoothSharkFront, actualFront, 0.08f));

            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), smoothSharkFront));
            glm::mat4 sm = glm::translate(glm::mat4(1.0f), flock.predator.position);
            sm *= glm::mat4(glm::vec4(right, 0), glm::vec4(glm::cross(smoothSharkFront, right), 0), glm::vec4(smoothSharkFront, 0), glm::vec4(0, 0, 0, 1));

            shader.setMat4("model", glm::scale(sm, glm::vec3(2.8f)));
            glDrawArrays(GL_PATCHES, 0, sV);

            // Higher probability of bubble spawning for the predator trail
            if (((float)rand() / RAND_MAX) < 0.18f) {
                bubbles.push_back({ flock.predator.position - actualFront * 5.0f, 3.0f, 5.0f });
            }
        }

        // --- Bubbles  ---
        // Using CPU-side lifetime management and GPU-side dynamic scaling
        shader.setBool("isShark", false);
        shader.setBool("isFish", false);
        glBindVertexArray(sphereVAO);
        shader.setVec3("color", glm::vec3(0.7f, 0.9f, 1.0f));
        for (int i = 0; i < (int)bubbles.size(); i++) {
            bubbles[i].life -= dt;
            if (bubbles[i].life <= 0) {
                bubbles.erase(bubbles.begin() + i);
                i--;
                continue;
            }
            bubbles[i].pos.y += bubbles[i].speed * dt;

            // Dynamic scaling tied to the particle's life value
            float s = 0.3f * (bubbles[i].life / 3.0f);
            shader.setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), bubbles[i].pos), glm::vec3(s)));
            glDrawArrays(GL_PATCHES, 0, bV);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}