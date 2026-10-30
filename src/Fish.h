#ifndef FISH_H
#define FISH_H

#include <glm/glm.hpp>
#include <vector>

class Fish {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 color; // Per-fish color variation
    float maxSpeed;
    float maxForce;

    Fish(glm::vec3 pos);
    void applyForce(glm::vec3 force);
    void calculateBehaviors(std::vector<Fish>& fishes, glm::vec3 predatorPos, const std::vector<glm::vec3>& obstacles);
    void update(float deltaTime);

    glm::vec3 separate(std::vector<Fish>& fishes);
    glm::vec3 align(std::vector<Fish>& fishes);
    glm::vec3 cohesion(std::vector<Fish>& fishes);
    glm::vec3 avoidObstacles(const std::vector<glm::vec3>& obstacles);
};

#endif