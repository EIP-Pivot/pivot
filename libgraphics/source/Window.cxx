
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/DebugMacros.hxx"

#include <Logger.hpp>
#include <stdexcept>

Window::Window()
{
    glfwInit();
    this->setErrorCallback(error_callback);
}

Window::~Window()
{
    if (window != nullptr) glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow(const unsigned &width, const unsigned &height, const std::string &n)
{
    if (windowName.empty()) windowName = std::move(n);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    if (!window) throw WindowError("Failed to create a GLFW window");
    this->setUserPointer(this);
    this->setKeyCallback(keyboard_callback);
    this->setCursorPosCallback(cursor_callback);
}

vk::SurfaceKHR Window::createSurface(const vk::Instance &instance)
{
    assert(window);
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw WindowError("failed to create Vulkan surface");
    }
    return surface;
}

void Window::setKeyPressCallback(Window::Key key, Window::KeyEvent event) { keyPressMap.insert({key, event}); }
void Window::setKeyReleaseCallback(Window::Key key, Window::KeyEvent event) { keyReleaseMap.insert({key, event}); }
void Window::setMouseMovementCallback(Window::MouseEvent event) { mouseCallback = event; }

void Window::setTitle(const std::string &t) noexcept
{
    windowName = t;
    if (window) glfwSetWindowTitle(window, windowName.c_str());
}

vk::Extent2D Window::getSize() const noexcept
{
    assert(window);
    auto s = updateSize();
    return {
        .width = static_cast<uint32_t>(s.x),
        .height = static_cast<uint32_t>(s.y),
    };
}

void Window::captureCursor(bool capture) noexcept
{
    assert(window);
    if (capture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

bool Window::captureCursor() noexcept
{
    assert(window);
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

std::vector<const char *> Window::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtentsions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return {glfwExtentsions, glfwExtentsions + glfwExtensionCount};
}

void Window::setKeyCallback(GLFWkeyfun &&f) noexcept
{
    assert(window);
    glfwSetKeyCallback(window, f);
}
void Window::setCursorPosCallback(GLFWcursorposfun &&f) noexcept
{
    assert(window);
    glfwSetCursorPosCallback(window, f);
}
void Window::setResizeCallback(GLFWwindowsizefun &&f) noexcept
{
    assert(window);
    glfwSetFramebufferSizeCallback(window, f);
}

void Window::setErrorCallback(GLFWerrorfun &&f) noexcept { glfwSetErrorCallback(f); }

void Window::setUserPointer(void *ptr) noexcept
{
    assert(window);
    glfwSetWindowUserPointer(window, ptr);
}

glm::ivec2 Window::updateSize() const noexcept
{
    assert(window);
    glm::ivec2 size;
    glfwGetFramebufferSize(window, &size.x, &size.y);
    return size;
};

void Window::error_callback(int code, const char *msg) noexcept { logger.err("Window") << code << ": " << msg; }

void Window::cursor_callback(GLFWwindow *win, double xpos, double ypos)
{
    auto window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    if (window->mouseCallback) (window->mouseCallback.value())(*window, glm::dvec2(xpos, ypos));
}

void Window::keyboard_callback(GLFWwindow *win, int key, int, int action, int)
{
    auto window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    auto _key = static_cast<Window::Key>(key);

    switch (action) {
        case GLFW_PRESS:
            if (window->keyPressMap.contains(_key)) window->keyPressMap.at(_key)(*window, _key);
            break;
        case GLFW_RELEASE:
            if (window->keyReleaseMap.contains(_key)) window->keyReleaseMap.at(_key)(*window, _key);
            break;
        default: break;
    }
}