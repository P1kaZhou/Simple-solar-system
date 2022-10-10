//
// Created by daniel on 10/10/22.
//

#ifndef TPOPENGL_CAMERA_H
#define TPOPENGL_CAMERA_H


#include "glm/vec3.hpp"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <cmath>

class Camera {
public:
    inline float getFov() const { return m_fov; }

    inline void setFoV(const float f) { m_fov = f; }

    inline float getAspectRatio() const { return m_aspectRatio; }

    inline void setAspectRatio(const float a) { m_aspectRatio = a; }

    inline float getNear() const { return m_near; }

    inline void setNear(const float n) { m_near = n; }

    inline float getFar() const { return m_far; }

    inline void setFar(const float n) { m_far = n; }

    inline void setPosition(const glm::vec3 &p) { m_pos = p; }

    inline glm::vec3 getPosition() { return m_pos; }

    inline glm::mat4 computeViewMatrix() const {
        return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }

    // Returns the projection matrix stemming from the camera intrinsic parameter.
    inline glm::mat4 computeProjectionMatrix() const {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
    }

private:
    glm::vec3 m_pos = glm::vec3(0, 0, 0);
    float m_fov = 45.f;        // Field of view, in degrees
    float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
    float m_near = 0.1f; // Distance before which geometry is excluded from the rasterization process
    float m_far = 10.f; // Distance after which the geometry is excluded from the rasterization process
};


#endif //TPOPENGL_CAMERA_H
