#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
		glm::vec3 cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        glm::vec3 cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        if (direction == MOVE_FORWARD) {
            cameraPosition += cameraFrontDirection * speed;
            cameraTarget += cameraFrontDirection * speed;
        }
        else if (direction == MOVE_BACKWARD) {
            cameraPosition -= cameraFrontDirection * speed;
            cameraTarget -= cameraFrontDirection * speed;
        }
        else if (direction == MOVE_RIGHT) {
            cameraPosition += cameraRightDirection * speed;
            cameraTarget += cameraRightDirection * speed;
        }
        else if (direction == MOVE_LEFT) {
            cameraPosition -= cameraRightDirection * speed;
            cameraTarget -= cameraRightDirection * speed;
		}
        else if (direction == MOVE_UP) {
            cameraPosition += cameraUpDirection * speed;
            cameraTarget += cameraUpDirection * speed;
        }
        else if (direction == MOVE_DOWN) {
            cameraPosition -= cameraUpDirection * speed;
            cameraTarget -= cameraUpDirection * speed;
		}
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
    }
    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }
}