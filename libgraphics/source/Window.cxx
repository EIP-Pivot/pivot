
#include "pivot/graphics/Window.hxx"
#include "pivot/graphics/DebugMacros.hxx"
#include <stdexcept>

Window::Window(std::string n, unsigned w, unsigned h): width(w), height(h), windowName(n) { initWindow(); }

Window::~Window()
{
    if (window != nullptr) glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::shouldClose() const noexcept { return glfwWindowShouldClose(window); }
void Window::shouldClose(bool bClose) const noexcept { glfwSetWindowShouldClose(window, bClose); }

vk::SurfaceKHR Window::createSurface(const vk::Instance &instance)
{
    DEBUG_FUNCTION
    VkSurfaceKHR surface{};
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create surface");
    }
    return surface;
}

void Window::setKeyEventCallback(Window::Key key, Window::KeyEvent event) { keyEventMap.insert({key, event}); }
void Window::setMouseMovementCallback(Window::MouseEvent event) { mouseCallback = event; }

void Window::setTitle(const std::string &t) noexcept
{
    windowName = t;
    glfwSetWindowTitle(window, windowName.c_str());
}

void Window::captureCursor(bool capture) noexcept
{
    if (capture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

int Window::getWidth() const noexcept
{
    updateSize();
    return width;
}

int Window::getHeight() const noexcept
{
    updateSize();
    return height;
}

vk::Extent2D Window::getSize() const noexcept
{
    updateSize();
    return {
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
    };
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

void Window::setUserPointer(void *ptr) noexcept { glfwSetWindowUserPointer(window, ptr); }

void Window::initWindow() noexcept
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    this->setUserPointer(this);
    this->setKeyCallback(keyboard_callback);
    this->setCursorPosCallback(cursor_callback);
}

void Window::updateSize() const noexcept { glfwGetFramebufferSize(window, &width, &height); };

void cursor_callback(GLFWwindow *win, double xpos, double ypos)
{
    auto window = (Window *)glfwGetWindowUserPointer(win);
    if (window->mouseCallback) (*window->mouseCallback)(*window, glm::dvec2(xpos, ypos));
}

void keyboard_callback(GLFWwindow *win, int key, int, int action, int)
{
    auto window = (Window *)glfwGetWindowUserPointer(win);
    auto _key = static_cast<Window::Key>(key);
    auto _action = static_cast<Window::KeyAction>(action);

    if (window->keyEventMap.contains(_key)) window->keyEventMap.at(_key)(*window, _key, _action);
}