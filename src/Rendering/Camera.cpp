#include "Camera.h"
#include <GLFW/glfw3.h> 
#include <glm/gtc/matrix_transform.hpp> 
#include <algorithm> 


Camera::Camera(int w, int h) : windowWidth(w), windowHeight(h) {
    viewOffset.x = -static_cast<float>(windowWidth) / (2.0f * currentZoomLevel);
    viewOffset.y = -static_cast<float>(windowHeight) / (2.0f * currentZoomLevel);

}
void Camera::toggleViewMode() {
    is3DView = !is3DView;
}

void Camera::reset3DView() {
    if (is3DView) {
        cameraDistance = 100.0f; 
        cameraYaw = 0.0f;
        cameraPitch = glm::radians(0.0f); 
    }
}

void Camera::handleScroll(GLFWwindow* window, double xoffset, double yoffset) {
    if (is3DView) {
        cameraDistance -= static_cast<float>(yoffset) * ZOOM_SENSITIVITY_3D;
        cameraDistance = glm::clamp(cameraDistance, MIN_CAMERA_DISTANCE_3D, MAX_CAMERA_DISTANCE_3D);
    } else {
        // Pobieranie pozycji kursora na ekranie
        double mouseX_screen, mouseY_screen;
        glfwGetCursorPos(window, &mouseX_screen, &mouseY_screen);
         // Obliczanie pozycji w świecie
        glm::vec2 screen_mouse_pos(static_cast<float>(mouseX_screen), static_cast<float>(windowHeight - mouseY_screen));
        glm::vec2 world_pos_under_cursor_before_zoom = screenToWorld2D(screen_mouse_pos);

        // Zoomowanie
        float oldZoomLevel = currentZoomLevel;
        if (yoffset > 0) {
            currentZoomLevel *= (1.0f + ZOOM_SENSITIVITY_2D * 2.0f);
        } else if (yoffset < 0) {
            currentZoomLevel /= (1.0f + ZOOM_SENSITIVITY_2D * 2.0f);
        }
        currentZoomLevel = glm::clamp(currentZoomLevel, MIN_ZOOM_LEVEL_2D, MAX_ZOOM_LEVEL_2D);

        if (oldZoomLevel == currentZoomLevel) return; 
        
        // Aktualizacja widoku po zoomie
        viewOffset = world_pos_under_cursor_before_zoom - (screen_mouse_pos / currentZoomLevel);
    }
}

void Camera::handleMouseButton(GLFWwindow* window, int button, int action) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (is3DView) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) isRotating3D = (action == GLFW_PRESS);
        if (isRotating3D) lastMousePos3D = glm::vec2(xpos, ypos);
    } else {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) isPanning2D = (action == GLFW_PRESS);
        if (isPanning2D) lastMousePos2D = glm::vec2(xpos, ypos);
    }
}

void Camera::handleMouseMove(GLFWwindow* window, double xpos, double ypos) {
    if (is3DView && isRotating3D) {
        glm::vec2 currentMousePos(xpos, ypos);
        glm::vec2 delta = currentMousePos - lastMousePos3D;
        lastMousePos3D = currentMousePos;
        cameraYaw -= delta.x * ROTATION_SENSITIVITY_3D;
        cameraPitch = glm::clamp(cameraPitch - delta.y * ROTATION_SENSITIVITY_3D, MIN_PITCH_3D, MAX_PITCH_3D);
    } else if (!is3DView && isPanning2D) {
        glm::vec2 currentMousePos(xpos, ypos);
        glm::vec2 delta = currentMousePos - lastMousePos2D;
        lastMousePos2D = currentMousePos;
        if (currentZoomLevel != 0.0f) { 
            viewOffset.x -= delta.x / currentZoomLevel;
            viewOffset.y += delta.y / currentZoomLevel; 
        }
    }
}

glm::mat4 Camera::getProjectionMatrix() const {
    if (is3DView) {
        return glm::perspective(glm::radians(45.0f),
                                static_cast<float>(windowWidth) / static_cast<float>(windowHeight),
                                0.1f, MAX_CAMERA_DISTANCE_3D); 
    } else {
        float viewW = static_cast<float>(windowWidth) / currentZoomLevel;
        float viewH = static_cast<float>(windowHeight) / currentZoomLevel;
        return glm::ortho(viewOffset.x, viewOffset.x + viewW,
                          viewOffset.y, viewOffset.y + viewH,
                          -10.0f, 10.0f); 
    }
}

glm::mat4 Camera::getViewMatrix() const {
    if (is3DView) {
        glm::vec3 offset;
        offset.x = cos(cameraPitch) * sin(cameraYaw);
        offset.y = cos(cameraPitch) * cos(cameraYaw);
        offset.z = sin(cameraPitch);

        glm::vec3 position = cameraTarget3D + cameraDistance * offset;
        return glm::lookAt(position, cameraTarget3D, cameraUp3D);
    } else {
        return glm::mat4(1.0f);
    }
}

glm::vec2 Camera::screenToWorld2D(const glm::vec2& screenPos) const {
    if (is3DView) {
        return glm::vec2(0.0f); 
    }
    if (currentZoomLevel == 0.0f) return viewOffset; 
    return viewOffset + (screenPos / currentZoomLevel);
}