
#include "pivot/graphics/Window.hxx"
#include "pivot/pivot.hxx"

#include <magic_enum.hpp>
#include <stb_image.h>
#include <stdexcept>

static int translate_key(int key, int scancode)
{
    // https://github.com/ocornut/imgui/blob/60bea052a92cbb4a93b221002fdf04f0da3698e1/backends/imgui_impl_glfw.cpp#L294
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL) return key;
    const char *key_name = glfwGetKeyName(key, scancode);
    if (key_name && key_name[0] != 0 && key_name[1] == 0) {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = {GLFW_KEY_GRAVE_ACCENT,  GLFW_KEY_MINUS,     GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET,
                                 GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_COMMA, GLFW_KEY_SEMICOLON,
                                 GLFW_KEY_APOSTROPHE,    GLFW_KEY_PERIOD,    GLFW_KEY_SLASH, 0};

        pivotAssertMsg(std::size(char_names) == std::size(char_keys), "Special caracters are not of the same size.");
        if (key_name[0] >= '0' && key_name[0] <= '9') {
            key = GLFW_KEY_0 + (key_name[0] - '0');
        } else if (key_name[0] >= 'A' && key_name[0] <= 'Z') {
            key = GLFW_KEY_A + (key_name[0] - 'A');
        } else if (key_name[0] >= 'a' && key_name[0] <= 'z') {
            key = GLFW_KEY_A + (key_name[0] - 'a');
        } else if (const char *p = std::strchr(char_names, key_name[0])) {
            key = char_keys[p - char_names];
        }
    }
    return key;
}

namespace pivot::graphics
{

Window::Window()
{
    DEBUG_FUNCTION();
    glfwInit();
    this->setErrorCallback(error_callback);
}

Window::~Window()
{
    DEBUG_FUNCTION();
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow(const std::string &name)
{
    DEBUG_FUNCTION();

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    initWindow(name, 600, 600);
}

void Window::initWindow(const std::string &name, unsigned width, unsigned height)
{
    PROFILE_FUNCTION();

    if (!verify(window)) { glfwDestroyWindow(window); }

    windowName = name;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    if (!window) throw WindowError("Failed to create a GLFW window !");

    this->setUserPointer(this);
    this->setKeyCallback(keyboard_callback);
    this->setCursorPosCallback(cursor_callback);
    this->setResizeCallback(resize_callback);
}

vk::SurfaceKHR Window::createSurface(const vk::Instance &instance)
{
    DEBUG_FUNCTION();
    pivotAssert(window);

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create surface");
    }
    return surface;
}

void Window::addResizeCallback(ResizeEvent event) { resizeEventCallback.push_back(event); }

void Window::addKeyPressCallback(Window::Key key, Window::KeyEvent event) { keyPressMap[key].push_back(event); }
void Window::addGlobalKeyPressCallback(Window::KeyEvent event) { globalKeyPressMap.push_back(event); }

void Window::addKeyReleaseCallback(Window::Key key, Window::KeyEvent event) { keyReleaseMap[key].push_back(event); }
void Window::addGlobalKeyReleaseCallback(Window::KeyEvent event) { globalKeyReleaseMap.push_back(event); }

void Window::addMouseMovementCallback(Window::MouseEvent event) { mouseCallback.push_back(event); }

void Window::setTitle(const std::string &t) noexcept
{
    pivotAssert(window);

    windowName = t;
    glfwSetWindowTitle(window, windowName.c_str());
}

void Window::setIcon(const std::span<const GLFWimage> &images) noexcept
{
    pivotAssert(window);
    glfwSetWindowIcon(window, images.size(), images.data());
}

void Window::setIcon(const std::span<const std::string> &windowIcons)
{
    DEBUG_FUNCTION();
    pivotAssert(window);

    std::vector<GLFWimage> images;
    for (const auto &icon: windowIcons) {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load(icon.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) throw std::runtime_error("Failed to load icon at path: " + icon);
        images.push_back({
            .width = texWidth,
            .height = texHeight,
            .pixels = pixels,
        });
    }
    setIcon(images);
    for (const auto &i: images) { stbi_image_free(i.pixels); }
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
    pivotAssert(window);

    if (capture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

bool Window::captureCursor() noexcept { return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED; }

std::vector<const char *> Window::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtentsions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return {glfwExtentsions, glfwExtentsions + glfwExtensionCount};
}

void Window::setKeyCallback(GLFWkeyfun &&f) noexcept
{
    pivotAssert(window);
    glfwSetKeyCallback(window, f);
}
void Window::setCursorPosCallback(GLFWcursorposfun &&f) noexcept
{
    pivotAssert(window);
    glfwSetCursorPosCallback(window, f);
}
void Window::setResizeCallback(GLFWwindowsizefun &&f) noexcept
{
    pivotAssert(window);
    glfwSetFramebufferSizeCallback(window, f);
}

void Window::setErrorCallback(GLFWerrorfun &&f) noexcept { glfwSetErrorCallback(f); }

void Window::setUserPointer(void *ptr) noexcept
{
    pivotAssert(window);
    glfwSetWindowUserPointer(window, ptr);
}

glm::ivec2 Window::updateSize() const noexcept
{
    glm::ivec2 size;
    glfwGetFramebufferSize(window, &size.x, &size.y);
    return size;
};

Window::Key Window::getTrueKey(const Window::Key &ex) const noexcept
{
    PROFILE_FUNCTION();
    auto key = magic_enum::enum_integer(ex);
    auto translate = translate_key(key, 0);
    auto true_key = magic_enum::enum_cast<Window::Key>(translate);
    if (true_key.has_value()) return true_key.value();
    return Key::UNKNOWN;
}

void Window::error_callback(int, const char *msg) noexcept { logger.err("Window") << msg; }

void Window::cursor_callback(GLFWwindow *win, double xpos, double ypos)
{
    auto window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    for (auto &&fn: window->mouseCallback) fn(*window, glm::dvec2(xpos, ypos));
}

void Window::keyboard_callback(GLFWwindow *win, int key, int scancode, int action, int modifier)
{
#define FOR_EACH(vec) \
    for (auto &&fn: vec) fn(*window, _key, _modifier);

    auto window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    auto _key = static_cast<Window::Key>(translate_key(key, scancode));
    auto _modifier = static_cast<Window::Modifier>(modifier);

    switch (action) {
        case GLFW_PRESS:
            FOR_EACH(window->globalKeyPressMap);
            if (window->keyPressMap.contains(_key)) FOR_EACH(window->keyPressMap.at(_key));
            break;
        case GLFW_RELEASE:
            FOR_EACH(window->globalKeyReleaseMap);
            if (window->keyReleaseMap.contains(_key))
                if (window->keyReleaseMap.contains(_key)) FOR_EACH(window->keyReleaseMap.at(_key));
            break;
        default: break;
    }
#undef FOR_EACH
}

void Window::resize_callback(GLFWwindow *win, int width, int height)
{
    auto window = static_cast<Window *>(glfwGetWindowUserPointer(win));
    for (auto &&fn: window->resizeEventCallback) fn(*window, glm::i32vec2(width, height));
}

}    // namespace pivot::graphics
