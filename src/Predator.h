#ifndef PREDATOR_H
#define PREDATOR_H

#include <glm/glm.hpp>
#include <vector>
#include "Fish.h"

class Predator {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float maxSpeed;
    int targetIndex;
    float targetTimer;

    Predator(glm::vec3 pos);
    
    void update(float deltaTime, std::vector<Fish>& fishes, const std::vector<glm::vec3>& obstacles);
};

#endif