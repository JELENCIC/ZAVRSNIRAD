#ifndef APLIKACIJA_H
#define APLIKACIJA_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Namespace for your app
namespace MyApp
{
    void InitializeCallbacks(GLFWwindow* window);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
    static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    static void MouseCallback(GLFWwindow* window, double xPos, double yPos);
    // Moves/alters the camera positions based on user input
    void DoMovement();
    
    // Function to initialize framebuffer
    void InitFramebuffer(int width, int height);

    void InitResources();

    // Function to render the scene to the framebuffer
    void RenderSceneToFramebuffer();

    // Function to render ImGui UI
    void RenderUI(GLFWwindow* window);

    void Initialize();
    // Function to set up the scene (load shaders, textures, etc.)
    void SetupScene();

    void Cleanup();

    // Variables for managing UI windows
    extern bool show_another_window1;
    extern bool show_another_window2;
    extern bool show_another_window3;
}

#endif // APLIKACIJA_H
