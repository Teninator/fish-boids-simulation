#include "Fish.h"
#include <glm/glm.hpp>
#include <cstdlib>

/**
   Constructor: Sets up the initial state and visual variety.
   This is the 'beef' of the vaporwave aesthetic is handled here with randomized lavender and purple hues to keep the school from looking uniform.
 */
Fish::Fish(glm::vec3 pos) : position(pos), acceleration(0.0f) {
    // Initial velocity is restricted to the XZ plane for a top-down focus
    velocity = glm::vec3((rand() % 10 - 5) / 5.0f, 0.0f, (rand() % 10 - 5) / 5.0f);
    maxSpeed = 12.0f;
    maxForce = 0.5f;

    // Vaporwave Variation: Skewing toward Lavender, Pink, and Light Purple
    float r = 0.6f + (rand() % 30) / 100.0f; // 0.6 to 0.9
    float g = 0.4f + (rand() % 30) / 100.0f; // 0.4 to 0.7
    float b = 0.8f + (rand() % 20) / 100.0f; // 0.8 to 1.0
    color = glm::vec3(r, g, b);
}

void Fish::applyForce(glm::vec3 force) {
    acceleration += force;
}

/**
   calculateBehaviors
   Orchestrates the weighted Boids forces and environmental reactions.
 */
void Fish::calculateBehaviors(std::vector<Fish>& fishes, glm::vec3 predatorPos, const std::vector<glm::vec3>& obstacles) {
    // Standard Boid weights: Separation is prioritized to prevent overlapping
    applyForce(separate(fishes) * 3.5f);
    applyForce(align(fishes) * 1.0f);
    applyForce(cohesion(fishes) * 0.2f);
    applyForce(avoidObstacles(obstacles));

    // Predator Evasion: Force magnitude increases as the predator gets closer
    float distToPred = glm::distance(position, predatorPos);
    if (distToPred < 45.0f) {
        glm::vec3 fleeDir = glm::normalize(position - predatorPos);
        // Using an inverse-distance multiplier for a sharper reaction
        applyForce(fleeDir * (55.0f / (distToPred + 0.1f)) * 12.0f);
    }

    // World Boundary: Gently pulls fish back if they stray past the 120 unit limit
    if (glm::length(position) > 120.0f) {
        applyForce(glm::normalize(-position) * 6.0f);
    }
}

/** Steering Behaviors: Separation, Alignment, Cohesion
   These functions implement Craig Reynolds' core flocking logic.
 */
glm::vec3 Fish::separate(std::vector<Fish>& fishes) {
    glm::vec3 steer(0.0f);
    int count = 0;
    for (auto& other : fishes) {
        float d = glm::distance(position, other.position);
        if (d > 0 && d < 12.0f) {
            steer += glm::normalize(position - other.position) / d;
            count++;
        }
    }
    return (count > 0) ? steer / (float)count : steer;
}

glm::vec3 Fish::align(std::vector<Fish>& fishes) {
    glm::vec3 sum(0.0f);
    int count = 0;
    for (auto& other : fishes) {
        float d = glm::distance(position, other.position);
        if (d > 0 && d < 20.0f) {
            sum += other.velocity;
            count++;
        }
    }
    return (count > 0) ? glm::normalize(sum) * maxForce : glm::vec3(0.0f);
}

glm::vec3 Fish::cohesion(std::vector<Fish>& fishes) {
    glm::vec3 sum(0.0f);
    int count = 0;
    for (auto& other : fishes) {
        float d = glm::distance(position, other.position);
        if (d > 0 && d < 20.0f) {
            sum += other.position;
            count++;
        }
    }
    return (count > 0) ? glm::normalize((sum / (float)count) - position) * maxForce : glm::vec3(0.0f);
}

/**
   avoidObstacles
   Implements a hybrid approach: smooth steering for distant obstacles and hard reflection/clamping for internal collisions.
 */
glm::vec3 Fish::avoidObstacles(const std::vector<glm::vec3>& obstacles) {
    glm::vec3 steer(0.0f);
    float coralRadius = 20.0f;
    float detectionBuffer = 28.0f;

    for (const auto& obs : obstacles) {
        glm::vec2 fishPos = glm::vec2(position.x, position.z);
        glm::vec2 obsPos = glm::vec2(obs.x, obs.z);
        float distance = glm::distance(fishPos, obsPos);

        // Hard collision: teleport fish to edge and reflect velocity
        if (distance < coralRadius) {
            glm::vec2 escapeDir = glm::normalize(fishPos - obsPos);
            glm::vec2 newPos2D = obsPos + (escapeDir * (coralRadius + 0.2f));
            position.x = newPos2D.x;
            position.z = newPos2D.y;

            glm::vec2 reflectedVel = glm::reflect(glm::vec2(velocity.x, velocity.z), escapeDir);
            velocity.x = reflectedVel.x * 0.5f; // Dampen bounce
            velocity.z = reflectedVel.y * 0.5f;
        }
        // Predictive steering: nudge fish away before they hit the radius
        else if (distance < detectionBuffer) {
            glm::vec2 steerDir = glm::normalize(fishPos - obsPos);
            float strength = (detectionBuffer - distance) / detectionBuffer;
            steer.x += steerDir.x * strength * 35.0f;
            steer.z += steerDir.y * strength * 35.0f;
        }
    }
    return steer;
}

/**
   update
   Standard Euler integration for movement. Acceleration is reset every frame to prevent force accumulation.
 */
void Fish::update(float deltaTime) {
    velocity += acceleration;

    // Clamp speed so things don't fly off screen
    if (glm::length(velocity) > maxSpeed) {
        velocity = glm::normalize(velocity) * maxSpeed;
    }

    position += velocity * deltaTime;

    // Reset acceleration for the next frame
    acceleration *= 0.0f;
}