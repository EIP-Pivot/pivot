#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <glm/vec2.hpp>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "pivot/exception.hxx"
#include "pivot/pivot.hxx"
#include "pivot/utility/flags.hxx"

namespace pivot::graphics
{
/// @class Window
///
/// @brief A class allowing some abstaction over the GLFW library
///
/// This class is designed to be used in conjuction with Vulkan API
/// @code
/// // Declare and create a new window
/// Window window("Test window", 200, 200);
///
/// // Set a callback on the Escape key
/// window.setKeyEventCallback(Window::Key::Escape,
///                            [](Window &window, const Window::Key key, const Window::KeyAction action) {
///                                 window.shouldClose(true);
///                             });
///
/// while(window.shouldClose()) {
///     window.pollEvent();
/// }
/// @endcode
class Window
{
public:
    /// Enum of the different key input
    enum class Key {
        UNKNOWN = GLFW_KEY_UNKNOWN,
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

    /// Represent the modifier key
    enum class ModifierBits : FlagsType {
        Alt = GLFW_MOD_ALT,
        Ctrl = GLFW_MOD_CONTROL,
        Shift = GLFW_MOD_SHIFT,
        Super = GLFW_MOD_SUPER,
    };
    /// Flag type
    using Modifier = Flags<ModifierBits>;

    /// Enum of the different key state
    enum class Action {
        Pressed = GLFW_PRESS,
        Release = GLFW_RELEASE,
    };

    /// Keyboard event callback signature
    using KeyEvent = std::function<void(Window &window, const Key key, const Modifier modifier)>;
    /// Mouse movement event callback signature
    using MouseEvent = std::function<void(Window &window, const glm::dvec2 pos)>;
    /// Resize event callback signature
    using ResizeEvent = std::function<void(Window &window, const glm::i32vec2 size)>;

    /// Error type for Window
    RUNTIME_ERROR(Window);

public:
    /// Create a new Window
    Window();
    Window(Window &) = delete;
    Window(const Window &) = delete;
    /// Destructor
    ~Window();

    /// Initialize the window to the maximum size
    void initWindow(const std::string &windowName);
    /// Initialiaze the window to the provided size
    void initWindow(const std::string &windowName, unsigned width, unsigned height);

    /// Return wether or not the window should be closed
    inline bool shouldClose() const noexcept { return glfwWindowShouldClose(window); }
    /// Mark the window as ready to be closed
    inline void shouldClose(bool bClose) const noexcept { glfwSetWindowShouldClose(window, bClose); }

    /// Poll new events
    ///
    /// - Should be called once per tick
    inline void pollEvent() noexcept { glfwPollEvents(); }

    /// Create a Vulkan surface
    ///
    /// @param instance a valid Vulkan instance, must have the required extention enabled, see
    /// Window::getRequiredExtensions()
    ///
    /// @return a valid vulkan surface, ready to be used. The Window class does not retain ownership of the surface
    vk::SurfaceKHR createSurface(const vk::Instance &instance);

    /// Tell wether or not a key is pressed
    ///
    /// @return true if the key is pressed, otherwise false
    FORCEINLINE bool isKeyPressed(Key _key) const noexcept
    {
        auto key = getTrueKey(_key);
        return glfwGetKey(this->window, static_cast<unsigned>(key)) == GLFW_PRESS;
    }
    /// Tell wether or not a key is not pressed
    ///
    /// @return true if the key is not pressed, otherwise false
    FORCEINLINE bool isKeyReleased(Key _key) const noexcept
    {
        auto key = getTrueKey(_key);
        return glfwGetKey(this->window, static_cast<unsigned>(key)) == GLFW_RELEASE;
    }

    /// Setup a callback function when the window is resized
    void addResizeCallback(ResizeEvent event);

    /// Setup a callback function for provided key when it is pressed
    ///
    /// @param key Which key this callback is listening to
    /// @param event The callback function to call when an event occur
    ///
    /// @see KeyEvent
    void addKeyPressCallback(Key key, KeyEvent event);

    /// Setup a callback function when a key is pressed
    ///
    /// @param event The callback function to call when an event occur
    ///
    /// @see KeyEvent
    void addGlobalKeyPressCallback(KeyEvent event);

    /// Setup a callback function for provided key when it is released
    ///
    /// @param key Which key this callback is listening to
    /// @param event The callback function to call when an event occur
    ///
    /// @see KeyEvent
    void addKeyReleaseCallback(Key key, KeyEvent event);

    /// Setup a callback function when a key is released
    ///
    /// @param event The callback function to call when an event occur
    ///
    /// @see KeyEvent
    void addGlobalKeyReleaseCallback(KeyEvent event);

    /// Setup a callback function for mouse movement
    /// @param event The callback function to call when the cursor move
    ///
    /// @see MouseEvent
    void addMouseMovementCallback(MouseEvent event);

    /// Set the title of the window
    /// @param title New title for the window
    void setTitle(const std::string &title) noexcept;

    /// Set the icon of the window
    void setIcon(const std::span<const GLFWimage> &) noexcept;
    /// Set the icon of the window
    void setIcon(const std::span<const std::string> &);

    /// Get the title of the window
    /// @return The title of the window
    constexpr const std::string &getTitle() const noexcept { return windowName; }

    /// get the width of the window
    inline int getWidth() const noexcept { return this->updateSize().x; }
    /// get the height of the window
    inline int getHeight() const noexcept { return this->updateSize().y; }

    /// get the size of the window
    vk::Extent2D getSize() const noexcept;

    /// Whether or not the window should capture the user's cursor
    /// @param bCapturing the window will capture the cursor if true
    void captureCursor(bool bCapturing) noexcept;
    /// Whether or not the window is capturing the user's cursor
    /// @return true if the cursor is captured, false otherwise
    bool captureCursor() noexcept;

    /// Return the raw GLFW window pointer
    /// @return a glfw window pointer
    constexpr GLFWwindow *getWindow() noexcept { return window; }

    /// Easy way to get the all the Vulkan extension required by GLFW
    /// @return a list of extension name required
    static std::vector<const char *> getRequiredExtensions();

private:
    void setKeyCallback(GLFWkeyfun &&f) noexcept;
    void setCursorPosCallback(GLFWcursorposfun &&f) noexcept;
    void setResizeCallback(GLFWwindowsizefun &&f) noexcept;
    void setErrorCallback(GLFWerrorfun &&f) noexcept;

    void setUserPointer(void *ptr) noexcept;
    glm::ivec2 updateSize() const noexcept;
    Key getTrueKey(const Key &ex) const noexcept;

    static void error_callback(int code, const char *msg) noexcept;
    static void cursor_callback(GLFWwindow *win, double xpos, double ypos);
    static void keyboard_callback(GLFWwindow *win, int key, int scancode, int action, int mods);
    static void resize_callback(GLFWwindow *win, int width, int height);

private:
    std::vector<MouseEvent> mouseCallback;
    std::vector<KeyEvent> globalKeyReleaseMap;
    std::vector<KeyEvent> globalKeyPressMap;
    std::vector<ResizeEvent> resizeEventCallback;

    std::unordered_map<Key, std::vector<KeyEvent>> keyPressMap;
    std::unordered_map<Key, std::vector<KeyEvent>> keyReleaseMap;

    std::string windowName;
    GLFWwindow *window = nullptr;
};
}    // namespace pivot::graphics

ENABLE_FLAGS_FOR_ENUM(pivot::graphics::Window::ModifierBits);
