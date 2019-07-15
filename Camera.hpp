#ifndef CAMERA_HPP
#define CAMERA_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>


class Camera
{
public:
    Camera();
    Camera(const glm::vec3 initialPosition, const float initialYaw, const float initialPitch);
    Camera& rotate(const float yaw, const float pitch);                                         // in degrees
    Camera& move(const float shiftX, const float shiftY, const float shiftZ);
    Camera& setPosition(const glm::vec3 position); 
    Camera& setRotation(const float yaw, const float pitch);                                    // in degrees

    const glm::vec3 getPosition() const;
    const glm::vec3 getNormalizedDirection() const;
private:
    glm::vec3 position;
    float yaw;
    float pitch;
};

#endif