#pragma once

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

class Window
{
public:
    enum class Key {
        A = GLFW_KEY_A,
        Z = GLFW_KEY_Z,
        E = GLFW_KEY_E,
        R = GLFW_KEY_R,
        T = GLFW_KEY_T,
        Y = GLFW_KEY_Y,
        U = GLFW_KEY_U,
        I = GLFW_KEY_I,
        O = GLFW_KEY_O,
        P = GLFW_KEY_P,
        Q = GLFW_KEY_Q,
        S = GLFW_KEY_S,
        D = GLFW_KEY_D,
        F = GLFW_KEY_F,
        G = GLFW_KEY_G,
        H = GLFW_KEY_H,
        J = GLFW_KEY_J,
        K = GLFW_KEY_K,
        L = GLFW_KEY_L,
        M = GLFW_KEY_M,
        W = GLFW_KEY_W,
        X = GLFW_KEY_X,
        C = GLFW_KEY_C,
        V = GLFW_KEY_V,
        B = GLFW_KEY_B,
        N = GLFW_KEY_N,
        RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
        LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
        RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
        LEFT_ALT = GLFW_KEY_LEFT_ALT,
        RIGHT_CTRL = GLFW_KEY_RIGHT_CONTROL,
        LEFT_CTRL = GLFW_KEY_LEFT_CONTROL,
        SPACE = GLFW_KEY_SPACE,
        ESCAPE = GLFW_KEY_ESCAPE,
        UP = GLFW_KEY_UP,
        DOWN = GLFW_KEY_DOWN,
        RIGHT = GLFW_KEY_RIGHT,
        LEFT = GLFW_KEY_LEFT,
    };
    enum class KeyAction {
        Pressed = GLFW_PRESS,
        Release = GLFW_RELEASE,
    };

    using KeyEvent = std::function<void(Window &window, const Key key, const KeyAction action)>;
    using MouseEvent = std::function<void(Window &window, const glm::dvec2 pos)>;

public:
    explicit Window(std::string, unsigned, unsigned);
    Window(Window &) = delete;
    Window(const Window &) = delete;
    ~Window();
    bool shouldClose() const noexcept;
    void shouldClose(bool bClose) const noexcept;
    inline void pollEvent() noexcept { glfwPollEvents(); }
    vk::SurfaceKHR createSurface(const vk::Instance &);
    inline bool isKeyPressed(unsigned key) const { return glfwGetKey(this->window, key) == GLFW_PRESS; }
    inline bool isKeyReleased(unsigned key) const { return glfwGetKey(this->window, key) == GLFW_RELEASE; }
    void setKeyEventCallback(Key key, KeyEvent event = {});
    void setMouseMovementCallback(MouseEvent event = {});

    void setTitle(const std::string &t) noexcept;
    constexpr const std::string &getTitle() const noexcept { return windowName; }

    int getWidth() const noexcept;
    int getHeight() const noexcept;
    vk::Extent2D getSize() const noexcept;

    void captureCursor(bool) noexcept;
    bool captureCursor() noexcept;

    static std::vector<const char *> getRequiredExtensions();

private:
    void setKeyCallback(GLFWkeyfun &&f) noexcept;
    void setCursorPosCallback(GLFWcursorposfun &&f) noexcept;
    void setResizeCallback(GLFWwindowsizefun &&f) noexcept;
    void setUserPointer(void *ptr) noexcept;
    void initWindow() noexcept;
    void updateSize() const noexcept;

    friend void cursor_callback(GLFWwindow *win, double xpos, double ypos);
    friend void keyboard_callback(GLFWwindow *win, int key, int, int action, int);

private:
    mutable int width;
    mutable int height;

    std::optional<MouseEvent> mouseCallback = {};
    std::unordered_map<Key, KeyEvent> keyEventMap;

    std::string windowName;
    GLFWwindow *window = nullptr;
};

void cursor_callback(GLFWwindow *win, double xpos, double ypos);
void keyboard_callback(GLFWwindow *win, int key, int, int action, int);
