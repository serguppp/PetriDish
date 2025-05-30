#include <glm/glm.hpp>

struct GLFWwindow;

class Camera {
public:
    // Rozmiar okna
    int windowWidth;
    int windowHeight;

    // Widok 2D/3D
    bool is3DView = false;

    // Konfiguracja widoku 2D
    float currentZoomLevel = 1.0f;
    glm::vec2 viewOffset{0.0f, 0.0f}; 
    glm::vec2 lastMousePos2D{0.0f, 0.0f};
    bool isPanning2D = false;

    // Konfiguracja widoku 3D
    glm::vec3 cameraTarget3D{0.0f, 0.0f, 0.0f};
    glm::vec3 cameraUp3D{0.0f, 0.0f, -1.0f}; 
    float cameraDistance = 100.0f;
    float cameraYaw = 0.0f;
    float cameraPitch = glm::radians(-90.0f);
    glm::vec2 lastMousePos3D{0.0f, 0.0f};
    bool isRotating3D = false;

    // Stałe dla kamery
    static constexpr float MAX_ZOOM_LEVEL_2D = 30.0f;
    static constexpr float MIN_ZOOM_LEVEL_2D = 0.2f;
    static constexpr float ZOOM_SENSITIVITY_2D = 0.1f;
    static constexpr float ROTATION_SENSITIVITY_3D = 0.005f;
    static constexpr float ZOOM_SENSITIVITY_3D = 1.5f;
    static constexpr float MIN_CAMERA_DISTANCE_3D = 5.0f;
    static constexpr float MAX_CAMERA_DISTANCE_3D = 5000.0f;
    static constexpr float MIN_PITCH_3D = glm::radians(-89.0f);
    static constexpr float MAX_PITCH_3D = glm::radians(89.0f);

    Camera(int w, int h);

    void toggleViewMode();
    void reset3DView(); 
    void handleScroll(GLFWwindow* window, double xoffset, double yoffset);
    void handleMouseButton(GLFWwindow* window, int button, int action);
    void handleMouseMove(GLFWwindow* window, double xpos, double ypos);

    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewMatrix() const;
    glm::vec2 screenToWorld2D(const glm::vec2& screenPos) const;
};

