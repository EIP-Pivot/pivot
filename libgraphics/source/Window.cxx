
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include <stdexcept>

Window::Window(std::string n, unsigned w, unsigned h): windowName(n) { initWindow(w, h); }

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

void Window::setKeyPressCallback(Window::Key key, Window::KeyEvent event) { keyPressMap.insert({key, event}); }
void Window::setKeyReleaseCallback(Window::Key key, Window::KeyEvent event) { keyReleaseMap.insert({key, event}); }
void Window::setMouseMovementCallback(Window::MouseEvent event) { mouseCallback = event; }

void Window::setTitle(const std::string &t) noexcept
{
    windowName = t;
    glfwSetWindowTitle(window, windowName.c_str());
}

vk::Extent2D Window::getSize() const noexcept
{
    auto s = updateSize();
    return {
        .width = static_cast<uint32_t>(s.x),
        .height = static_cast<uint32_t>(s.y),
    };
}

void Window::captureCursor(bool capture) noexcept
{
    if (capture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

bool Window::captureCursor() noexcept { return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED; }

// static
std::vector<const char *> Window::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtentsions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtentsions, glfwExtentsions + glfwExtensionCount);
    return extensions;
}

void Window::setKeyCallback(GLFWkeyfun &&f) noexcept { glfwSetKeyCallback(window, f); }
void Window::setCursorPosCallback(GLFWcursorposfun &&f) noexcept { glfwSetCursorPosCallback(window, f); }
void Window::setResizeCallback(GLFWwindowsizefun &&f) noexcept { glfwSetFramebufferSizeCallback(window, f); }

void Window::setErrorCallback(GLFWerrorfun &&f) noexcept { glfwSetErrorCallback(f); }

void Window::setUserPointer(void *ptr) noexcept { glfwSetWindowUserPointer(window, ptr); }

void Window::initWindow(const unsigned width, const unsigned height) noexcept
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->setErrorCallback(error_callback);
    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    this->setUserPointer(this);
    this->setKeyCallback(keyboard_callback);
    this->setCursorPosCallback(cursor_callback);
}

glm::ivec2 Window::updateSize() const noexcept
{
    glm::ivec2 size;
    glfwGetFramebufferSize(window, &size.x, &size.y);
    return size;
};

void Window::error_callback(int code, const char *msg) noexcept
{
    logger->err("Window") << msg;
    LOGGER_ENDL;
}

void cursor_callback(GLFWwindow *win, double xpos, double ypos)
{
    auto window = (Window *)glfwGetWindowUserPointer(win);
    if (window->mouseCallback) (*window->mouseCallback)(*window, glm::dvec2(xpos, ypos));
}

void keyboard_callback(GLFWwindow *win, int key, int, int action, int)
{
    auto window = (Window *)glfwGetWindowUserPointer(win);
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