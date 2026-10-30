#ifndef FLOCK_H
#define FLOCK_H

#include <vector>
#include <glm/glm.hpp>
#include "Fish.h"
#include "Predator.h"

class Flock {
public:
    Flock(int fishCount);
    void update(float deltaTime, const std::vector<glm::vec3>& obstacles);

    void avoidObstacles(const std::vector<glm::vec3>& obstacles);

    std::vector<Fish> fishes;
    Predator predator;
};

#endif