#include "Camera.hpp"

namespace gps
{

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::cross(cameraFrontDirection, cameraUpDirection);

        //TODO - Update the rest of camera parameters
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction)
        {
        case MOVE_LEFT:
            this->cameraPosition -= cameraRightDirection * speed;
            break;
        case MOVE_RIGHT:
            this->cameraPosition += cameraRightDirection * speed;
            break;
        case MOVE_FORWARD:
            this->cameraPosition += cameraFrontDirection * speed;
            break;
        case MOVE_BACKWARD:
            this->cameraPosition -= cameraFrontDirection * speed;
            break;
        case MOVE_DOWN:
            this->cameraPosition.y -= speed; 
            break;   
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw)
    {
        // calculate the new Front vector
        glm::vec3 front;
        cameraFrontDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection.y = sin(glm::radians(pitch));
        cameraFrontDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        //cameraFrontDirection = glm::normalize(front);
        // also re-calculate the Right and Up vector
        cameraTarget = cameraPosition + cameraFrontDirection;
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0, 1.0, 0.0))); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
} // namespace gps