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
    GLFWwindow *getWindow() { return window; }
    inline bool shouldClose() const { return glfwWindowShouldClose(window); }
    inline void pollEvent() { glfwPollEvents(); }
    vk::SurfaceKHR createSurface(const vk::Instance &);
    inline bool isKeyPressed(unsigned key) const { return glfwGetKey(this->window, key) == GLFW_PRESS; }

    void setKeyCallback(GLFWkeyfun &&f);
    void setCursorPosCallback(GLFWcursorposfun &&f);
    void setResizeCallback(void(&&)(GLFWwindow *, int, int));

    void unsetKeyCallback();
    void unsetCursorPosCallback();
    void captureCursor(bool);
    void setUserPointer(void *ptr);
    void setTitle(const std::string &t);
    const std::string &getTitle() const { return windowName; }

    unsigned getWidth() const { return width; }
    unsigned getHeight() const { return height; }
    vk::Extent2D getSize() const
    {
        return {
            .width = width,
            .height = height,
        };
    }

    static std::vector<const char *> getRequiredExtensions();

private:
    void initWindow();

private:
    unsigned width;
    unsigned height;
    std::string windowName;
    GLFWwindow *window = nullptr;
};
