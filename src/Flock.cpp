#include "Flock.h"
#include <cstdlib>

/**
   Flock Constructor
   Initializes the school with a specified population. The fish are spawned in a randomized 3D volume to ensure the simulation doesn't start with all agents occupying the same spatial coordinates.
 */
Flock::Flock(int fishCount) : predator(glm::vec3(0.0f, 0.0f, -40.0f)) {
    for (int i = 0; i < fishCount; i++) {
        // Random distribution within a 120x20x120 area
        fishes.push_back(Fish(glm::vec3((rand() % 120) - 60, (rand() % 20) - 10, (rand() % 120) - 60)));
    }
}

/**
 * update
   The main simulation loop for the entire flock. This manages the order of operations: predator movement -> boid logic -> environmental avoidance.
 */
void Flock::update(float deltaTime, const std::vector<glm::vec3>& obstacles) {
    // Updates the predator first so fish react to its new position immediately
    predator.update(deltaTime, fishes, obstacles);

    // Calculate the "intent" of each fish based on neighbor and predator positions
    for (auto& f : fishes) {
        f.calculateBehaviors(fishes, predator.position, obstacles);
    }

    // Apply the Top-Down Force Field as an override to standard behaviors
    avoidObstacles(obstacles);

    // Finalize movement by integrating velocity into position
    for (auto& f : fishes) {
        f.update(deltaTime);
    }
}

/**
   avoidObstacles
   Implements the Top-Down Force Field logic. Instead of checking full 3D spheres, this treats obstacles as infinite vertical pillars on the XZ plane to prevent clipping from the camera's perspective.
 */
void Flock::avoidObstacles(const std::vector<glm::vec3>& obstacles) {
    float avoidanceRadius = 22.0f;
    float maxForce = 18.0f; // High magnitude to ensure complete avoidance

    for (auto& f : fishes) {
        for (const auto& coralPos : obstacles) {
            // Projects to 2D coordinates for pillar avoidance
            glm::vec2 fishP(f.position.x, f.position.z);
            glm::vec2 coralP(coralPos.x, coralPos.z);

            float d = glm::distance(fishP, coralP);

            // If a fish enters the avoidance zone, applies an exponential push-back
            if (d < avoidanceRadius && d > 0.1f) {
                glm::vec2 push = glm::normalize(fishP - coralP);

                // Exponential strength ensures the force ramps up as distance decreases
                float strength = pow(1.0f - (d / avoidanceRadius), 2.0f);

                f.velocity.x += push.x * strength * maxForce;
                f.velocity.z += push.y * strength * maxForce;

                // Braking factor (0.96) prevents the fish from "slingshotting" out of the avoidance zone at unnatural speeds.
                f.velocity *= 0.96f;
            }
        }
    }
}