#include "Predator.h"
#include <glm/glm.hpp>

/**
   Predator Constructor
   Initializes the shark with a default heading and a target timer to manage its hunting state.
 */
Predator::Predator(glm::vec3 pos) : position(pos), targetIndex(-1), targetTimer(0.0f) {
    velocity = glm::vec3(0.0f, 0.0f, 1.0f);
    maxSpeed = 16.0f; // Significantly faster than the boids to maintain pressure
}

void Predator::update(float deltaTime, std::vector<Fish>& fishes, const std::vector<glm::vec3>& obstacles) {
    if (fishes.empty()) return;
    targetTimer -= deltaTime;

    /**
       Target Commitment Logic
       To prevent the shark from "jittering" between multiple nearby fish, it commits to a single target for a few seconds or until the target is lost/invalid.
     */
    if (targetIndex == -1 || targetIndex >= (int)fishes.size() || targetTimer <= 0.0f) {
        float minD = 9999.0f;
        for (int i = 0; i < (int)fishes.size(); i++) {
            float d = glm::distance(this->position, fishes[i].position);
            if (d < minD) {
                minD = d;
                targetIndex = i;
            }
        }
        // Commit to this prey for 4 seconds before re-evaluating
        targetTimer = 4.0f;
    }

    /**
        SMOOTH STEERING (The Jaggedness Fix)
        Instead of snapping directly to the target, we blend the current heading with the desired heading. This simulates physical mass and momentum.
     */
    glm::vec3 targetPos = fishes[targetIndex].position;
    glm::vec3 desiredDir = glm::normalize(targetPos - this->position);

    // Shark like turning radius.
    float blendingFactor = 0.04f;

    glm::vec3 currentDir = glm::normalize(this->velocity);
    // Linear interpolation (mix) creates a smooth arc during pursuit
    glm::vec3 smoothDir = glm::normalize(glm::mix(currentDir, desiredDir, blendingFactor));

    this->velocity = smoothDir * maxSpeed;

    /**
       Coral Collision
       Treats corals as solid cylinders. If the shark hits one, it snaps to the exterior and slides along the tangent instead of stopping.
     */
    for (const auto& obs : obstacles) {
        glm::vec2 p2D = glm::vec2(this->position.x, this->position.z);
        glm::vec2 o2D = glm::vec2(obs.x, obs.z);
        float d = glm::distance(p2D, o2D);
        float sharkRadius = 26.0f; // Larger radius than fish to reflect the model's scale

        if (d < sharkRadius) {
            // Snaps position to the boundary of the obstacle
            glm::vec2 push = glm::normalize(p2D - o2D);
            glm::vec2 snappedPos = o2D + (push * (sharkRadius + 0.5f));
            this->position.x = snappedPos.x;
            this->position.z = snappedPos.y;

            // Projects velocity onto the obstacle tangent so the shark doesn't lose all momentum on impact.
            glm::vec2 vel2D = glm::vec2(this->velocity.x, this->velocity.z);
            if (glm::dot(vel2D, -push) > 0) {
                glm::vec2 slideVel = vel2D - glm::dot(vel2D, -push) * -push;
                this->velocity.x = slideVel.x;
                this->velocity.z = slideVel.y;
            }
        }
    }

    // Final integration of velocity into position
    this->position += this->velocity * deltaTime;
}