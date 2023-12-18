#include <glad/glad.h>
#include <GLFW/glfw3.h>
// GLM library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

// OpenGL Sample Project
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// OpenGL Sample Project
// camera
Camera camera(glm::vec3(0.0f, 0.2f, 1.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Mathematical constants
const double PI = 3.14159;
const float toRadians = PI / 180.0f;

bool isOrtho = true;

// Array for triangle rotations
glm::float32 triRotations[] = { 0.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f };

// Vertex and fragment shader source code
// I modified both the vertex and fragment shader using code from loeanOpenGL coordinate section (ourShader "6.2.coordinate_system.vs")
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"    // LoeanOpenGL coordinate section 
"out vec3 ourColor;\n"
"out vec2 TexCoord;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
"   ourColor = aColor;\n"
"   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"   // LoeanOpenGL coordinate section 
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"in vec2 TexCoord;\n"   // LoeanOpenGL coordinate section 
"uniform sampler2D myTexture;\n"
"void main()\n"
"{\n"
//"   FragColor = vec4(ourColor, 1.0f);\n"
"   FragColor = texture(myTexture, TexCoord);\n"
"}\n\0";



int main()
{
    // GLFW initialization and configuration
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Jarrale Butts", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // OpenGL Sample Project
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth testing
    // Help for this code was retrieved from the Advanced OpenGL (Depth Testing) section in learnOpenGL
    glEnable(GL_DEPTH_TEST);

    // Build and Compile Shader Program
    // ------------------------------------
    // Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link Shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Get Uniform Locations for Cube and Pyramid
    unsigned int cubeModelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int cubeViewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int cubeProjectionLoc = glGetUniformLocation(shaderProgram, "projection");

    unsigned int pyramidModelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int pyramidViewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int pyramidProjectionLoc = glGetUniformLocation(shaderProgram, "projection");



    float cylinderVertices[] = {
        // position attributes (x, y, z), texture coordinates (u, v)
        0.0f, 0.0f, 0.0f,    0.5f, 0.5f, // vert 1
        0.5f, 0.866f, 0.0f,  1.0f, 0.0f, // vert 2
        1.0f, 0.0f, 0.0f,    1.0f, 1.0f, // vert 3
        0.5f, 0.866f, 0.0f,  1.0f, 0.0f, // vert 4
        0.5f, 0.866f, -2.0f, 0.0f, 0.0f, // vert 5
        1.0f, 0.0f, 0.0f,    1.0f, 1.0f, // vert 6
        1.0f, 0.0f, 0.0f,    1.0f, 1.0f, // vert 7
        1.0f, 0.0f, -2.0f,   0.0f, 1.0f, // vert 8
        0.5f, 0.866f, -2.0f, 0.0f, 0.0f // vert 9
    };

    // Leg Vertex
    float legVertices[] = {
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

    // Pyramid Vertex
    // This shape was taken from my previous project
    float pyramidVertices[] = {
         0.1f,  0.1f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // bottom right 0  Blue
         0.1f, -0.1f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom left 1   Yellow
        -0.1f, -0.1f, 0.0f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f, // top left 2      Pink
        -0.1f,  0.1f, 0.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, // top right 3     Teal
         0.0f,  0.0f, 0.1f,  1.0f, 1.0f, 1.0f,  0.5f, 0.5f  // Center          White
    };

    // Rectangle Vertex
    // These verticies will initally be used for my plane
    float rectangleVerticies[] = {
        // Top face
        -0.1f, -0.1f, 0.1f,  1.0f, 0.0f, 0.0f, // 0
         0.1f, -0.1f, 0.1f,  0.0f, 1.0f, 0.0f, // 1
         0.1f,  0.1f, 0.1f,  1.0f, 1.0f, 0.0f, // 2
        -0.1f,  0.1f, 0.1f,  1.0f, 1.0f, 0.0f, // 3

        // Bottom face
        -0.1f, -0.1f, -0.1f, 0.0f, 0.0f, 1.0f, // 4
         0.1f, -0.1f, -0.1f, 0.0f, 0.0f, 1.0f, // 5
         0.1f,  0.1f, -0.1f, 0.0f, 1.0f, 1.0f, // 6
        -0.1f,  0.1f, -0.1f, 0.0f, 1.0f, 1.0f, // 7
    };

    // Pyramid Indices
    unsigned int pyramidIndices[] = {
       0, 1, 3,  // first Triangle 
       1, 2, 3,  // second Triangle 
       2, 3, 4,  // third Triangle
       3, 0, 4,  // fourth Triangle 
       0, 1, 4,  // fifth Triangle 
       1, 2, 4   // sixth Triangle 
    };

    // Rectangle Indices
    unsigned int rectangleIndices[] = {
        0, 1, 2, // Front face
        2, 3, 0,

        1, 5, 6, // Right face
        6, 2, 1,

        7, 6, 5, // Back face
        5, 4, 7,

        4, 0, 3, // Left face
        3, 7, 4,

        4, 5, 1,  // Bottom face
        1, 0, 4,

        3, 2, 6,  // Top face
        6, 7, 3
    };


    // Cylinder VAO, VBO setup
    unsigned int cylinderVAO, cylinderVBO;
    glGenVertexArrays(1, &cylinderVAO); // Create VAO
    glGenBuffers(1, &cylinderVBO); // Create VBO

    glBindVertexArray(cylinderVAO); // Activate VAO for VBO association
    glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO); // Enable VBO	
    glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderVertices), cylinderVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO with VA (Vertex Attribute)
    glEnableVertexAttribArray(0); // Enable VA

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO with VA
    glEnableVertexAttribArray(1); // Enable VA
    glBindVertexArray(0);


    // Leg VAO, VBO setup
    unsigned int legVAO, legVBO;
    glGenVertexArrays(1, &legVAO);
    glGenBuffers(1, &legVBO);

    glBindVertexArray(legVAO);
    glBindBuffer(GL_ARRAY_BUFFER, legVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(legVertices), legVertices, GL_STATIC_DRAW);

    // Position Attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture Attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);


    // Pyramid VAO, VBO, and EBO setup
    unsigned int pyramidVAO, pyramidVBO, pyramidEBO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glGenBuffers(1, &pyramidEBO);

    glBindVertexArray(pyramidVAO);

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    // Position Attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color Attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture Attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);


    // Rectangle VAO, VBO, and EBO setup
    unsigned int rectangleVAO, rectangleVBO, rectangleEBO;
    glGenVertexArrays(1, &rectangleVAO);
    glGenBuffers(1, &rectangleVBO);
    glGenBuffers(1, &rectangleEBO);

    glBindVertexArray(rectangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVerticies), rectangleVerticies, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectangleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices), rectangleIndices, GL_STATIC_DRAW);
    // Position Attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color Attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);


    ////////////////// Load texture //////////////////
    unsigned int leg;
    glGenTextures(1, &leg);
    glBindTexture(GL_TEXTURE_2D, leg);
    // Set wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load and generate the texture
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("telescopeLeg.jpeg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    unsigned int peg;
    glGenTextures(1, &peg);
    glBindTexture(GL_TEXTURE_2D, peg);
    // Set texture parameters (wrapping, filtering)
    // Load and generate the texture
    int width2, height2, nrChannels2;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data2 = stbi_load("blackPlastic.jpeg", &width2, &height2, &nrChannels2, 0);
    if (data2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data2);


    // Set the texture unit in the shader
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture"), 0);

    // Wireframe Polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Render Loop
    while (!glfwWindowShouldClose(window))
    {
        // OpenGL Sample Project
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Rendering setup
        // Help for this code was retrieved from the Advanced OpenGL (Depth Testing) section in learnOpenGL
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Shader program use
        glUseProgram(shaderProgram);

        // View and projection matrices
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // OpenGLSample-2Objects
        //        glm::mat4 projection = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.1f, 100.0f); // OpenGL (Camera) section in learnOpenGL

        
                glm::mat4 projection;
                if (isOrtho) {
                    // OpenGL (Camera) section in learnOpenGL
                    projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
                }
                else {
                    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
                }
        
        glm::mat4 view = camera.GetViewMatrix();

        /*
                // Draw Leg
                glBindTexture(GL_TEXTURE_2D, leg);
                glBindVertexArray(legVAO);
                glm::mat4 modelLeg = glm::mat4(1.0f);
                // (set up model matrix for cube, pass transformations to the shader)
                modelLeg = glm::translate(modelLeg, glm::vec3(0.0f, -0.1f, 1.0f)); // Move the object
                modelLeg = glm::rotate(modelLeg, 90.0f * toRadians, glm::vec3(-1.0f, 0.0f, 0.0f)); // Add rotation
                //modelCube = glm::rotate(modelCube, 0.0f * toRadians, glm::vec3(0.0f, 0.0f, -1.0f)); // Add rotation
                modelLeg = glm::scale(modelLeg, glm::vec3(0.2f, 0.2f, 0.5f)); // Add scaling
                // Pass Transformations to the Shader
                // CS-330-master (Module 03 , Tutorial_03_05)
                glUniformMatrix4fv(cubeModelLoc, 1, GL_FALSE, glm::value_ptr(modelLeg));
                glUniformMatrix4fv(cubeViewLoc, 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(cubeProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
                glDrawArrays(GL_TRIANGLES, 0, 36);
        */
        // Draw Leg
        glBindTexture(GL_TEXTURE_2D, leg);
        glBindVertexArray(legVAO);
        //glm::mat4 modelLeg = glm::mat4(1.0f);
        for (int i = 0; i < 3; ++i) {
            glm::mat4 triPod = glm::mat4(1.0f); // Copy the base model matrix

            // Modify modelLeg3 based on location (example)
            if (i == 0) {
                triPod = glm::translate(triPod, glm::vec3(-0.1f, 0.155f, 0.1f)); // Move the object
                triPod = glm::rotate(triPod, 25.0f * toRadians, glm::vec3(0.0f, 0.0f, -1.0f)); // Add rotation
                triPod = glm::scale(triPod, glm::vec3(0.02f, 0.3f, 0.02f)); // Add scaling
            }
            else if (i == 1) {
                triPod = glm::translate(triPod, glm::vec3(0.0f, 0.155f, -0.01f)); // Move the object
                triPod = glm::rotate(triPod, 25.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // Add rotation
                triPod = glm::scale(triPod, glm::vec3(0.02f, 0.3f, 0.02f)); // Add scaling
            }
            else {
                triPod = glm::translate(triPod, glm::vec3(0.1f, 0.155f, 0.1f)); // Move the object
                triPod = glm::rotate(triPod, 25.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f)); // Add rotation
                triPod = glm::scale(triPod, glm::vec3(0.02f, 0.3f, 0.02f)); // Add scaling
            }

            // Pass the modified model matrix to the shader
            glUniformMatrix4fv(cubeModelLoc, 1, GL_FALSE, glm::value_ptr(triPod));

            // Draw the object
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // Unbind Shader exe and VOA after drawing per frame
        glBindVertexArray(0); //Incase different VAO wii be used after
        glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture


        // Draw Pyramid
        glBindTexture(GL_TEXTURE_2D, peg);
        glBindVertexArray(pyramidVAO);
        for (int i = 0; i < 3; ++i) {
            glm::mat4 pegs = glm::mat4(1.0f); // Copy the base model matrix

            // Modify modelLeg3 based on location (example)
            if (i == 0) {
                pegs = glm::translate(pegs, glm::vec3(-0.163f, 0.02f, 0.1f)); // Move the object
                pegs = glm::rotate(pegs, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // Add rotation
                pegs = glm::rotate(pegs, 25.0f * toRadians, glm::vec3(0.0f, -1.0f, 0.0f)); // Add rotation
                pegs = glm::scale(pegs, glm::vec3(0.11f, 0.11f, 0.1f)); // Add scaling
            }
            else if (i == 1) {
                pegs = glm::translate(pegs, glm::vec3(0.0f, 0.02f, -0.073f)); // Move the object
                pegs = glm::rotate(pegs, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // Add rotation
                pegs = glm::rotate(pegs, 25.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // Add rotation
                pegs = glm::scale(pegs, glm::vec3(0.11f, 0.11f, 0.1f)); // Add scaling
            }
            else {
                pegs = glm::translate(pegs, glm::vec3(0.163f, 0.02f, 0.1f)); // Move the object
                pegs = glm::rotate(pegs, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f)); // Add rotation
                pegs = glm::rotate(pegs, 25.0f * toRadians, glm::vec3(1.0f, 1.0f, 0.0f)); // Add rotation
                pegs = glm::scale(pegs, glm::vec3(0.11f, 0.11f, 0.1f)); // Add scaling
            }

            // Pass the modified model matrix to the shader
            glUniformMatrix4fv(pyramidModelLoc, 1, GL_FALSE, glm::value_ptr(pegs));

            // Draw the pyramid
            glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
        }
        // Unbind Shader exe and VOA after drawing per frame
        glBindVertexArray(0); //Incase different VAO wii be used after
        glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture


        glBindVertexArray(cylinderVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()

        // Use loop to build Model matrix for triangle
        for (int i = 0; i < 6; i++) {

            glm::mat4 tube = glm::mat4(1.0f);
            // (set up model matrix for cube, pass transformations to the shader)
            tube = glm::translate(tube, glm::vec3(0.0f, 0.32f, 0.16f)); // Move the object
            tube = glm::rotate(tube, glm::radians(-10.f), glm::vec3(1.0f, 0.0f, 0.0f));
            tube = glm::rotate(tube, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate strip on z by increments in array
            tube = glm::scale(tube, glm::vec3(0.05f, 0.05f, 0.1f)); // Add scaling
            // Pass Transformations to the Shader
            // CS-330-master (Module 03 , Tutorial_03_05)
            glUniformMatrix4fv(cubeModelLoc, 1, GL_FALSE, glm::value_ptr(tube));
            glUniformMatrix4fv(cubeViewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(cubeProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glDepthFunc(GL_LEQUAL);
            glDrawArrays(GL_TRIANGLES, 0, 9);

        }
        // Unbind Shader exe and VOA after drawing per frame
        glBindVertexArray(0); //Incase different VAO wii be used after
        glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture


        glBindVertexArray(cylinderVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()
        // Use loop to build Model matrix for triangle
        for (int i = 0; i < 6; i++) {

            glm::mat4 eye = glm::mat4(1.0f);
            // (set up model matrix for cube, pass transformations to the shader)
            eye = glm::translate(eye, glm::vec3(0.065f, 0.35f, 0.15f)); // Move the object
            eye = glm::rotate(eye, glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.0f));
            eye = glm::rotate(eye, glm::radians(60.f), glm::vec3(0.0f, 1.0f, 0.0f));
            eye = glm::rotate(eye, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate strip on z by increments in array
            eye = glm::scale(eye, glm::vec3(0.008f, 0.008f, 0.015f)); // Add scaling
            // Pass Transformations to the Shader
            // CS-330-master (Module 03 , Tutorial_03_05)
            glUniformMatrix4fv(cubeModelLoc, 1, GL_FALSE, glm::value_ptr(eye));
            glUniformMatrix4fv(cubeViewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(cubeProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glDepthFunc(GL_LEQUAL);
            glDrawArrays(GL_TRIANGLES, 0, 9);
        }
        // Unbind Shader exe and VOA after drawing per frame
        glBindVertexArray(0); //Incase different VAO wii be used after
        glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture


        glBindVertexArray(cylinderVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()
        // Use loop to build Model matrix for triangle
        for (int i = 0; i < 6; i++) {

            glm::mat4 scope = glm::mat4(1.0f);
            // (set up model matrix for cube, pass transformations to the shader)
            scope = glm::translate(scope, glm::vec3(-0.045f, 0.35f, 0.16f)); // Move the object
            scope = glm::rotate(scope, glm::radians(-10.f), glm::vec3(1.0f, 0.0f, 0.0f));
            scope = glm::rotate(scope, glm::radians(triRotations[i]), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate strip on z by increments in array
            scope = glm::scale(scope, glm::vec3(0.008f, 0.008f, 0.015f)); // Add scaling
            // Pass Transformations to the Shader
            // CS-330-master (Module 03 , Tutorial_03_05)
            glUniformMatrix4fv(cubeModelLoc, 1, GL_FALSE, glm::value_ptr(scope));
            glUniformMatrix4fv(cubeViewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(cubeProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glDepthFunc(GL_LEQUAL);
            glDrawArrays(GL_TRIANGLES, 0, 9);
        }
        // Unbind Shader exe and VOA after drawing per frame
        glBindVertexArray(0); //Incase different VAO wii be used after
        glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture


        // Draw Plane
        glBindVertexArray(rectangleVAO);
        glm::mat4 modelPlane = glm::mat4(1.0f);
        // (set up model matrix for cube, pass transformations to the shader)
        modelPlane = glm::rotate(modelPlane, 90.0f * toRadians, glm::vec3(-1.0f, 0.0f, 0.0f)); // Add rotation
        modelPlane = glm::scale(modelPlane, glm::vec3(3.0f, 3.0f, 0.1f)); // Add scaling
        // Pass Transformations to the Shader
        // CS-330-master (Module 03 , Tutorial_03_05)
        glUniformMatrix4fv(cubeModelLoc, 1, GL_FALSE, glm::value_ptr(modelPlane));
        glUniformMatrix4fv(cubeViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(cubeProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        // Unbind the VAO
        glBindVertexArray(0);

        // swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Unbind Shader exe and VOA after drawing per frame
    glBindVertexArray(0); //Incase different VAO wii be used after
    glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture


    // Deallocate resources
    glDeleteVertexArrays(1, &legVAO);
    glDeleteBuffers(1, &legVBO);

    glDeleteVertexArrays(1, &cylinderVAO);
    glDeleteBuffers(1, &cylinderVBO);

    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteBuffers(1, &pyramidVBO);
    glDeleteBuffers(1, &pyramidEBO);

    glDeleteVertexArrays(1, &rectangleVAO);
    glDeleteBuffers(1, &rectangleVBO);
    glDeleteBuffers(1, &rectangleEBO);

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// OpenGL (Camera) section in learnOpenGL
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        isOrtho = !isOrtho; // Toggle between ortho and perspective views
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// OpenGL (Camera) section in learnOpenGL
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // Check if the left mouse button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is 
// OpenGL (Camera) section in learnOpenGL
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}