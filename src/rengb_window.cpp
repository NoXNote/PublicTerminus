#include "rengb_window.hpp"

#include <stdexcept>

namespace rengb
{
    RengbWindow::RengbWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name}
    {
        initWindow();
    }

    RengbWindow::~RengbWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void RengbWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void RengbWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void RengbWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto rengbWindow =  reinterpret_cast<RengbWindow *>(glfwGetWindowUserPointer(window));
        rengbWindow->framebufferResized = true;
        rengbWindow->width = width;
        rengbWindow->height = height;
    }
    
}