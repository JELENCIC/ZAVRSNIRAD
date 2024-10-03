#include "Aplikacija.h"
#include "imgui.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
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
    bool AddingCube = false;
    bool RemovingCube = false;

    GLuint framebuffer, textureColorbuffer, rbo;
    
    Shader* ourShader;

    int currentFramebufferWidth = 1280;
    int currentFramebufferHeight = 720;

    

    std::vector<std::string> LoadTextureFiles(const std::string& directory) {
        std::vector<std::string> textureFiles;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file() && (entry.path().extension() == ".png" || entry.path().extension() == ".jpg")) {
                    textureFiles.push_back(entry.path().filename().string());
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error reading directory: " << e.what() << std::endl;
        }

        return textureFiles;
    }

    class Cube {
    public:
        GLuint VAO, VBO;
        GLuint texture;  // Store single texture ID for this cube
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotationAxis;
        GLfloat rotationAngle;

        Cube(glm::vec3 pos, glm::vec3 scl, glm::vec3 rotAxis, GLfloat rotAngle, GLuint tex)
            : position(pos), scale(scl), rotationAxis(rotAxis), rotationAngle(rotAngle), texture(tex) {}

        void InitResources() {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            
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

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(2);
            glBindVertexArray(0);
        }

        void Render(Shader& shader) const {
            shader.Use();
            glActiveTexture(GL_TEXTURE0);  // Activate texture unit
            glBindTexture(GL_TEXTURE_2D, texture);  // Bind the texture
            glUniform1i(glGetUniformLocation(shader.Program, "texture1"), 0);  // Assuming the shader uses "texture1"

            // Bind your VAO and draw the cube
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36); // Adjust if cube has different vertices
            glBindVertexArray(0);
        }

        void Cleanup() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteTextures(1, &texture);  // Cleanup single texture
        }
    };

    std::vector<Cube> cubes;
    std::vector<std::string> textureList;
    std::string textureFolder = "../texture"; // Folder path
    int selectedTextureIndex = 0;

    // Function to load textures from the texture folder
    GLuint LoadTexture(const std::string& filePath) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load texture image
        int texWidth, texHeight;
        unsigned char* image = SOIL_load_image(filePath.c_str(), &texWidth, &texHeight, 0, SOIL_LOAD_RGBA);

        if (image) {
            std::cout << "Loaded texture: " << filePath << std::endl;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
        }

        SOIL_free_image_data(image);
        glBindTexture(GL_TEXTURE_2D, 0);  // Unbind the texture
        return textureID;
    }


    
    glm::vec3 newCubePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 newCubeScale(1.0f, 1.0f, 1.0f);
    glm::vec3 newCubeRotationAxis(0.0f, 1.0f, 0.0f);
    float newCubeRotationAngle = 180.0f;
    

    void AddCube(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotationAxis, float rotationAngle, const std::string& textureName) {
        GLuint textureID = LoadTexture(textureFolder + "/" + textureName);  // Load the texture for this cube
        Cube newCube(position, scale, rotationAxis, rotationAngle, textureID);
        newCube.InitResources();  // Initialize resources for the new cube
        cubes.push_back(newCube); // Add to the cubes vector
    }

    void RemoveCube(int index) {
        if (index >= 0 && index < cubes.size()) {
            cubes.erase(cubes.begin() + index);
        }
    }


    // Camera
    Camera  camera(glm::vec3(0.0f, 3.0f, 0.0f));
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

    /*void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
        camera.ProcessMouseScroll(yOffset);
    }*/

    void MouseCallback(GLFWwindow* window, double xPos, double yPos)
    {
        if (Enable_Mouse_Rotation)  // Only process mouse movements if options is false
        {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
                glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
            {
                // Hide the cursor
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                static bool firstMouse = true; // Make sure to track the first mouse movement
                static GLfloat lastX = 400.0;   // Initialize with the middle of the window
                static GLfloat lastY = 300.0;   // Initialize with the middle of the window

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
        //glfwSetScrollCallback(window, ScrollCallback);
  
    }

    //std::vector<std::string> textureList;
    //std::string textureFolder = "../texture"; // Adjust this to your folder path
    //int selectedTextureIndex = -1;
    void InitResources() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ourShader = new Shader("core.vs", "core.frag");  // Initialize the Shader dynamically
        
        textureList = LoadTextureFiles(textureFolder);
        
    }
    glm::vec3 backgroundColor(0.1f, 0.1f, 0.1f); // Default background color (dark gray)
    const glm::vec3 defaultBackgroundColor(0.1f, 0.1f, 0.1f); // Default background color
    int selectedCube = -1;
    // Function to render the scene to the framebuffer
    void RenderSceneToFramebuffer()
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Bind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Poll events and handle movement
        glfwPollEvents();
        DoMovement();

        // Clear the framebuffer with a dark color and enable depth testing
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);  // Enable depth testing

        // Calculate aspect ratio and adjust viewport
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

        
        // Set up projection matrix (Perspective projection)
        glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)newWidth / (GLfloat)newHeight, 0.1f, 100.0f);
        // Set up view matrix (camera position)
        glm::mat4 view = camera.GetViewMatrix();

        // Get uniform locations
        GLint modelLoc = glGetUniformLocation(ourShader->Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader->Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader->Program, "projection");

        // Pass matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        
        

        
        for (auto& cube : cubes) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cube.position);
            model = glm::rotate(model, glm::radians(cube.rotationAngle), cube.rotationAxis);
            model = glm::scale(model, cube.scale);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            cube.Render(*ourShader); // Render the cube
        }
        

        // Unbind framebuffer
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
        ImGui::Begin("Tools", 0, 0);
        if (ImGui::Button(u8"\uf1b2")) { AddingCube = true; }
        if (ImGui::Button(u8"\uf047")) { RemovingCube = true; }
        ImGui::End();

        


        

        if (options)
        {
            bool closeOptions = options;  // Local variable to prevent closing the window unexpectedly

            ImGui::Begin("OPTIONS", &closeOptions, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);

            // Control the enabling/disabling of mouse rotation
            ImGui::Checkbox("Enable Mouse Rotation", &Enable_Mouse_Rotation);

            // Add a color picker to change the background color
            ImGui::ColorEdit3("Background Color", glm::value_ptr(backgroundColor)); // ColorEdit3 takes an array of 3 floats (RGB)

            // Add a button to reset the background color to its default value
            if (ImGui::Button("Reset Color to Default")) {
                backgroundColor = defaultBackgroundColor;  // Reset to default color
            }

            options = closeOptions;
            if (ImGui::Button("Close"))
            {
                options = false;  // Close the window
            }

            ImGui::End();
        }


        
        if (AddingCube)
        {
            // Create an ImGui window for adding cubes
            ImGui::Begin("Create Cube", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);

            ImGui::Text("Cube Properties");

            // Cube position
            ImGui::Text("Position");
            ImGui::InputFloat3("##Position", glm::value_ptr(newCubePosition));

            // Cube scale
            ImGui::Text("Scale");
            ImGui::InputFloat3("##Scale", glm::value_ptr(newCubeScale));

            // Rotation axis
            ImGui::Text("Rotation Axis");
            ImGui::InputFloat3("##Rotation Axis", glm::value_ptr(newCubeRotationAxis));

            // Rotation angle
            ImGui::Text("Rotation Angle");
            ImGui::SliderFloat("##Rotation Angle", &newCubeRotationAngle, 0.0f, 360.0f);

            // Texture selection
            ImGui::Text("Select Texture");

            // Ensure that there are textures in the list before accessing it
            if (!textureList.empty())
            {
                // Create a temporary array of const char*
                std::vector<const char*> textureNames;
                for (const auto& name : textureList) {
                    textureNames.push_back(name.c_str());
                }

                // Display the combo box
                ImGui::Combo("##Textures", &selectedTextureIndex, textureNames.data(), textureNames.size());
            }
            else
            {
                ImGui::Text("No textures available.");
            }

            if (ImGui::Button("Add")) {
                AddCube(newCubePosition, newCubeScale, newCubeRotationAxis, newCubeRotationAngle, textureList[selectedTextureIndex]);
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) AddingCube = false;

            ImGui::End();
        }
        if (RemovingCube)
        {
            ImGui::Begin("Cube Manipulation", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking);

            // Show the cube list in the ImGui list box.
            ImGui::Text("Cubes");
            if (ImGui::BeginListBox("##CubeList")) {  // Use an ID to differentiate the list box
                for (size_t i = 0; i < cubes.size(); i++) {
                    std::string cubeLabel = "Cube " + std::to_string(i);  // Label each cube by its index.
                    if (ImGui::Selectable(cubeLabel.c_str(), selectedCube == i)) {
                        selectedCube = i;  // Update the selected cube's index when clicked.
                    }
                }
                ImGui::EndListBox();

                // Check if a cube is selected for manipulation
                if (selectedCube >= 0 && selectedCube < cubes.size()) {
                    Cube& cube = cubes[selectedCube]; // Reference to the selected cube

                    // Cube position
                    ImGui::Text("Position");
                    ImGui::InputFloat3("##Position", glm::value_ptr(cube.position));

                    // Cube scale
                    ImGui::Text("Scale");
                    ImGui::InputFloat3("##Scale", glm::value_ptr(cube.scale));

                    // Rotation axis
                    ImGui::Text("Rotation Axis");
                    ImGui::InputFloat3("##Rotation Axis", glm::value_ptr(cube.rotationAxis));

                    // Rotation angle
                    ImGui::Text("Rotation Angle");
                    ImGui::SliderFloat("##Rotation Angle", &cube.rotationAngle, 0.0f, 360.0f);
                }
                else {
                    ImGui::Text("Select a cube to modify");
                }
            }

            // Provide a button to remove the selected cube.
            if (ImGui::Button("Remove")) {
                if (selectedCube >= 0 && selectedCube < cubes.size()) {
                    RemoveCube(selectedCube);

                    // If the last cube was removed, adjust the selected index.
                    if (selectedCube >= cubes.size()) {
                        selectedCube = cubes.size() - 1;
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) RemovingCube = false;

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
            // Update framebuffer dimensions
            currentFramebufferWidth = windowWidth;
            currentFramebufferHeight = windowHeight;

            // Delete and recreate the framebuffer
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
    void Cleanup() {
        for (auto& cube : cubes) {
            cube.Cleanup();
        }
        cubes.clear();
        delete ourShader;
    }

}
