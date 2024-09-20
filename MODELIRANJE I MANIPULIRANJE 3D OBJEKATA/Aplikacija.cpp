#include "Aplikacija.h"
#include "imgui.h"
#include "Fizika.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "SOIL2/SOIL2.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"

namespace MyApp
{
    bool Enable_Mouse_Rotation = false;
    bool options = true;
    bool show_another_window1 = false;
    bool show_another_window2 = false;
    bool show_another_window3 = false;

    GLuint framebuffer, textureColorbuffer, rbo;
    GLuint VAO, VBO, EBO, texture;
    Shader* ourShader;

    int currentFramebufferWidth = 1280;
    int currentFramebufferHeight = 720;

    
    void RenderInfiniteGrid(float spacing, int gridSize) {
        glLineWidth(1.0f); // Set line width

        // Draw the grid lines
        glBegin(GL_LINES);

        // Loop through X axis lines
        for (int i = -gridSize; i <= gridSize; ++i) {
            if (i == 0) {
                // X axis (red)
                glColor3f(1.0f, 0.0f, 0.0f);
            }
            else {
                // Other lines (white)
                glColor3f(1.0f, 1.0f, 1.0f);
            }

            glVertex2f(i * spacing, -gridSize * spacing); // Vertical line from bottom to top
            glVertex2f(i * spacing, gridSize * spacing);
        }

        // Loop through Y axis lines
        for (int i = -gridSize; i <= gridSize; ++i) {
            if (i == 0) {
                // Y axis (blue)
                glColor3f(0.0f, 0.0f, 1.0f);
            }
            else {
                // Other lines (white)
                glColor3f(1.0f, 1.0f, 1.0f);
            }

            glVertex2f(-gridSize * spacing, i * spacing); // Horizontal line from left to right
            glVertex2f(gridSize * spacing, i * spacing);
        }

        glEnd();
    }
    

    // Camera
    Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
    GLfloat lastX = currentFramebufferWidth / 2.0;
    GLfloat lastY = currentFramebufferHeight / 2.0;
    bool keys[1024];
    bool firstMouse = true;

    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    // Function prototypes
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (key >= 0 && key < 1024)
        {
            if (action == GLFW_PRESS)
            {
                keys[key] = true;
            }
            else if (action == GLFW_RELEASE)
            {
                keys[key] = false;
            }
        }
    }

    void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
        camera.ProcessMouseScroll(yOffset);
    }

    void MouseCallback(GLFWwindow* window, double xPos, double yPos)
    {
        if (Enable_Mouse_Rotation)  // Only process mouse movements if options is false
        {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                // Hide the cursor
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                if (firstMouse)
                {
                    lastX = xPos;
                    lastY = yPos;
                    firstMouse = false;
                }

                GLfloat xOffset = xPos - lastX;
                GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

                lastX = xPos;
                lastY = yPos;

                camera.ProcessMouseMovement(xOffset, yOffset);
            }
            else
            {
                // Show the cursor again when the left mouse button is released
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                firstMouse = true;  // Reset mouse tracking for next time
            }
        }
    }

    void DoMovement()
    {
        // Camera controls
        if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
        {
            camera.ProcessKeyboard(FORWARD, deltaTime);
        }

        if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
        {
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        }

        if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
        {
            camera.ProcessKeyboard(LEFT, deltaTime);
        }

        if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
        {
            camera.ProcessKeyboard(RIGHT, deltaTime);
        }
    }

    // Initialize framebuffer with the given width and height
    void InitFramebuffer(int width, int height)
    {
        currentFramebufferWidth = width;
        currentFramebufferHeight = height;

        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Create texture for framebuffer
        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

        // Create renderbuffer for depth and stencil
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind the framebuffer
    }

    void InitializeCallbacks(GLFWwindow* window)
    {
        // Set the required callback functions
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, MouseCallback);
        glfwSetScrollCallback(window, ScrollCallback);
  
    }
    glm::vec3 cubePositions[] =
    {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };
    // Initialize resources (Shaders, VAOs, VBOs, etc.)
    void InitResources()
    {
        glEnable(GL_DEPTH_TEST);

        // enable alpha support
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ourShader = new Shader("core.vs", "core.frag");  // Initialize the Shader dynamically


        GLfloat vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };

        

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

        //position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        //color attribute
        /*glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);*/
        //texture attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        // Load and create texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int texWidth, texHeight;
        unsigned char* image = SOIL_load_image("../texture/container2.png", &texWidth, &texHeight, 0, SOIL_LOAD_RGBA);

        if (image == nullptr) {
            std::cout << "Failed to load texture: " << SOIL_last_result() << std::endl;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        /*glm::mat4 projection;
        projection = glm::perspective(45.0f, (GLfloat)currentFramebufferWidth / currentFramebufferHeight, 0.1f, 1000.0f);*/
    }

    // Function to render the scene to the framebuffer
    void RenderSceneToFramebuffer()
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Bind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Check and call events
        glfwPollEvents();
        DoMovement();

        // Clear the framebuffer with a dark color and enable depth testing
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Calculate the viewport and set it
        float aspectRatio = 16.0f / 9.0f;
        int newWidth = currentFramebufferWidth;
        int newHeight = static_cast<int>(currentFramebufferWidth / aspectRatio);

        if (newHeight > currentFramebufferHeight)
        {
            newHeight = currentFramebufferHeight;
            newWidth = static_cast<int>(currentFramebufferHeight * aspectRatio);
        }

        int viewportX = (currentFramebufferWidth - newWidth) / 2;
        int viewportY = (currentFramebufferHeight - newHeight) / 2;
        glViewport(viewportX, viewportY, newWidth, newHeight);

        
        // Activate shader
        ourShader->Use();

        // Example of rendering an infinite grid with a large size
        float gridSpacing = 1.0f;  // Distance between grid lines
        int gridSize = 1000;       // Large enough to feel infinite
        // Set up the orthographic projection
        
        RenderInfiniteGrid(gridSpacing, gridSize);
        // Set up projection matrix (Perspective projection)
        //glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)newWidth / (GLfloat)newHeight, 0.1f, 100.0f);
        glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)newWidth / (GLfloat)newHeight, 0.1f, 100.0f);
        // Set up view matrix (camera position)
        glm::mat4 view = glm::mat4(1.0f);
        //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); // Move back by 3 units to view the object
        view = camera.GetViewMatrix();
        // Set up model matrix (rotating the object)
        glm::mat4 model;
        //model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

        // Get uniform locations
        GLint modelLoc = glGetUniformLocation(ourShader->Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader->Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader->Program, "projection");

        // Pass matrices to the shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(ourShader->Program, "ourTexture1"), 0);

        // Bind VAO and draw the cube
        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        for (GLuint i = 0; i < 10; i++)
        {
            // Calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            GLfloat angle = 1.0f * i;
            model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // Unbind the framebuffer to render to the screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Render the UI with docking and other controls
    void RenderUI(GLFWwindow* window)
    {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // Window options and docking setup
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Docking space
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        // Menus and options (as per your existing code)
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, true);}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Options")) { options = true; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Sidebar buttons and windows
        ImGui::Begin("Alati", 0, 0);
        if (ImGui::Button("T")) { show_another_window1 = true; }
        if (ImGui::Button("L")) { show_another_window2 = true; }
        if (ImGui::Button("P")) { show_another_window3 = true; }
        ImGui::End();

        if (options)
        {
            bool closeOptions = options;  // Local variable to prevent closing the window unexpectedly

            ImGui::Begin("OPTIONS", &closeOptions, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);

            // Control the enabling/disabling of mouse rotation
            ImGui::Checkbox("Enable Mouse Rotation", &Enable_Mouse_Rotation);

            options = closeOptions;
            if (ImGui::Button("Zatvori"))
            {
                options = false;  // Close the window
            }
            ImGui::End();    
        }


        if (show_another_window1)
        {
            ImGui::Begin("Ime novog prozora", &show_another_window1, 1);
            ImGui::Text("Koordinate tocke:");
            if (ImGui::Button("Zatvori")) { show_another_window1 = false; }
            ImGui::End();
        }
        if (show_another_window2)
        {
            ImGui::Begin("Linija", &show_another_window2, 1);
            ImGui::Text("Opis novog prozora 2");
            if (ImGui::Button("Zatvori")) show_another_window2 = false;
            ImGui::End();
        }
        if (show_another_window3)
        {
            ImGui::Begin("Površina", &show_another_window3, 1);
            ImGui::Text("Opis novog prozora 3");
            if (ImGui::Button("Zatvori")) show_another_window3 = false;
            ImGui::End();
        }

        // Viewport rendering
        ImGui::Begin("Viewport");

        glm::vec3 camPos = camera.getPosition();  // Get the camera's position
        ImGui::Text("Camera Position: X: %.2f, Y: %.2f, Z: %.2f", camPos.x, camPos.y, camPos.z);
        
        // Get the size of the available content region inside the window
        ImVec2 availableRegion = ImGui::GetContentRegionAvail();
        int windowWidth = (int)availableRegion.x;
        int windowHeight = (int)availableRegion.y;

        // Only resize framebuffer if window size changes
        if (windowWidth != currentFramebufferWidth || windowHeight != currentFramebufferHeight) {
            // Set the window size if it's not docked
            if (ImGui::IsWindowDocked()) {
                // Optionally do something else if docked
            }
            else {
                ImGui::SetWindowSize("Viewport", ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
            }
        }
        // Only resize framebuffer if window size changes
        if (windowWidth != currentFramebufferWidth || windowHeight != currentFramebufferHeight) {
            glDeleteTextures(1, &textureColorbuffer);
            glDeleteRenderbuffers(1, &rbo);
            glDeleteFramebuffers(1, &framebuffer);
            InitFramebuffer(windowWidth, windowHeight);
        }

        // Render scene to framebuffer
        RenderSceneToFramebuffer();

        // Display the framebuffer texture inside ImGui window
        ImGui::Image((void*)(intptr_t)textureColorbuffer, ImVec2((float)currentFramebufferWidth, (float)currentFramebufferHeight), ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End(); // End of Viewport window
        ImGui::End(); // End of DockSpace window
    }

    // Call InitResources at startup
    void Initialize()
    {
        InitResources();
        InitFramebuffer(currentFramebufferWidth, currentFramebufferHeight);
    }

    // Call Cleanup when done
    void Cleanup()
    {
        delete ourShader;  // Free the Shader object
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteRenderbuffers(1, &rbo);
    }

}
