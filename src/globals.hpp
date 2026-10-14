struct KeyMappings {
    int shoot = GLFW_MOUSE_BUTTON_LEFT;
    int sights = GLFW_MOUSE_BUTTON_RIGHT;
    int moveLeft = GLFW_KEY_A;
    int moveRight = GLFW_KEY_D;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int jump = GLFW_KEY_SPACE;
    int crouch = GLFW_KEY_LEFT_CONTROL;
    int sprint = GLFW_KEY_F;
    int diagnosis = GLFW_KEY_T;
    int whatever = GLFW_KEY_C;
};
inline int const gravity = 20;
inline KeyMappings const keys;