#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan.hpp>

class Window
{
public:
    Window(std::string, unsigned, unsigned);
    Window(Window &) = delete;
    Window(const Window &) = delete;
    ~Window();
    constexpr GLFWwindow *getWindow() noexcept { return window; }
    inline bool shouldClose() const noexcept { return glfwWindowShouldClose(window); }
    inline void pollEvent() noexcept { glfwPollEvents(); }
    vk::SurfaceKHR createSurface(const vk::Instance &);
    inline bool isKeyPressed(unsigned key) const { return glfwGetKey(this->window, key) == GLFW_PRESS; }

    void setKeyCallback(GLFWkeyfun &&f) noexcept;
    void setCursorPosCallback(GLFWcursorposfun &&f) noexcept;
    void setResizeCallback(GLFWwindowsizefun &&f) noexcept;

    void unsetKeyCallback() noexcept;
    void unsetCursorPosCallback() noexcept;
    void captureCursor(bool) noexcept;
    void setUserPointer(void *ptr) noexcept;
    void setTitle(const std::string &t) noexcept;
    constexpr const std::string &getTitle() const noexcept { return windowName; }

    constexpr unsigned getWidth() const noexcept { return width; }
    constexpr unsigned getHeight() const noexcept { return height; }
    constexpr vk::Extent2D getSize() const noexcept
    {
        return {
            .width = width,
            .height = height,
        };
    }

    static std::vector<const char *> getRequiredExtensions();

private:
    void initWindow() noexcept;

private:
    unsigned width;
    unsigned height;
    std::string windowName;
    GLFWwindow *window = nullptr;
};
