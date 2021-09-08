
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include <stdexcept>

Window::Window(std::string n, unsigned w, unsigned h): width(w), height(h), windowName(n) { initWindow(); }

Window::~Window()
{
    if (window != nullptr) glfwDestroyWindow(window);
    glfwTerminate();
}

vk::SurfaceKHR Window::createSurface(const vk::Instance &instance)
{
    DEBUG_FUNCTION
    VkSurfaceKHR surface{};
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create surface");
    }
    return surface;
}

void Window::initWindow() noexcept
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

void Window::setKeyCallback(GLFWkeyfun &&f) noexcept { glfwSetKeyCallback(window, f); }
void Window::setCursorPosCallback(GLFWcursorposfun &&f) noexcept { glfwSetCursorPosCallback(window, f); }
void Window::setResizeCallback(GLFWwindowsizefun &&f) noexcept { glfwSetFramebufferSizeCallback(window, f); }
void Window::captureCursor(bool capture) noexcept
{
    if (capture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Window::setUserPointer(void *ptr) noexcept { glfwSetWindowUserPointer(window, ptr); }

void Window::setTitle(const std::string &t) noexcept
{
    windowName = t;
    glfwSetWindowTitle(window, windowName.c_str());
}

std::vector<const char *> Window::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtentsions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtentsions, glfwExtentsions + glfwExtensionCount);
    return extensions;
}
