#include"Camera.hpp"

Camera::Camera(): position({0, 0, 0}), yaw(0), pitch(0) {}

Camera::Camera(const glm::vec3 initialPosition, const float initialYaw, const float initialPitch): position(initialPosition), yaw(initialYaw), pitch(initialPitch) {}

Camera& Camera::rotate(const float yaw, const float pitch)
{
    this->yaw += yaw;
    this->pitch += pitch;
    if(this->pitch < -89.0) this->pitch = -89.0;
    if(this->pitch > 89.0) this->pitch = 89.0;

    return *this;
}

Camera& Camera::move(const float shiftX, const float shiftY, const float shiftZ)
{
    position.x += shiftX;
    position.y += shiftY;
    position.z += shiftZ;

    return *this;
}

Camera& Camera::setPosition(const glm::vec3 position)
{
    this->position = position;

    return *this;
}

Camera& Camera::setRotation(const float yaw, const float pitch)
{
    this->yaw = yaw;
    this->pitch = pitch;
    if(this->pitch < -89.0) this->pitch = -89.0;
    if(this->pitch > 89.0) this->pitch = 89.0;

    return *this;
}

const glm::vec3 Camera::getPosition() const
{
    return position;
}

const glm::vec3 Camera::getNormalizedDirection() const
{
    glm::vec3 direction;
    direction.x = glm::cos(glm::radians(pitch)) * glm::cos(glm::radians(yaw));
    direction.y = -glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw));
    direction.z = -glm::sin(glm::radians(pitch));
    return glm::normalize(direction);
}