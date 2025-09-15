#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "../header/Shader.h"
#include "../header/stb_image.h"
#include"../header/Camera.h"
#include "../header/Model.h"
#include "../header/Mesh.h"
#include "../header/Arrow.h"
#include "../header/ShaderPath.h"

enum RenderMode {
    DEFAULT,
    REFLECTIVE,
    NORMAL_DISPLAY,
    DEBUG
};

//Forward Declare
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void generateSphere(float radius, int sectorCount, int stackCount,
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void renderPointLights(Shader& lightShader, int numLights, unsigned int& lightVAO);
void renderFloor(Shader& floorShader, unsigned int& planeVAO);
void setUpMVP(glm::mat4& view, glm::mat4& projection, glm::mat4& model, glm::mat4& lightProjection, glm::mat4& lightView, glm::mat4& lightSpaceMatrix);
void createDepthCubeMapTransforms(float near_plane, float far_plane, glm::vec3 lightPos, std::vector<glm::mat4>& shadowTransforms);
void loadPointLightsToShader(Shader& shader, const int numLights);
void generateObjectPositions(std::vector<glm::vec3>& objectPositions);
void renderQuad(const unsigned int quadVAO);

//texture paths
std::string texturePath = "resources/textures/";

std::string containerDiffusePath = texturePath + "containerDiffuse.png";
const char* containerDiffusePathCstr = containerDiffusePath.c_str();

std::string containerSpecularPath = texturePath + "containerSpecular.png";
const char* containerSpecularPathCstr = containerSpecularPath.c_str();

std::string floorDiffusePath = texturePath + "wood_floor_diff_4k.jpg";
const char* floorDiffusePathCstr = floorDiffusePath.c_str();

//sky box
std::vector<std::string> faces
{
    texturePath + "skybox/right.jpg",
    texturePath + "skybox/left.jpg",
    texturePath + "skybox/top.jpg",
    texturePath + "skybox/bottom.jpg",
    texturePath + "skybox/front.jpg",
    texturePath + "skybox/back.jpg",
};

//model paths
std::string modelPath = "resources/models/";
std::string backpackPath = modelPath + "backpack/backpack.obj";
const char* backpackPathCstr = backpackPath.c_str();

//Constants
#define PI 3.14159265f
float cubeVertices[] = {
    // positions          // normals
    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    // Left face
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

    // Right face
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

     // Bottom face
     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

     // Top face
     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

float planeVertices[] = {
    // positions            // normals         // texcoords
    -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
     25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
};

const unsigned int windowWidth = 800;
const unsigned int windowHeight = 600;

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

float windowAspect = (float)windowWidth / (float)windowHeight;
float shadowAspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;

//Camera Vars
float fov = 45.0f;
float near = 0.1f;
float far = 100.0f;
const float cameraSpeed = 0.2f;
Camera mCamera(glm::vec3(0.0f, 0.0f, 3.0f));

//normalDisplay Magnitude
float magnitude = 0.4f;

//DeltaTime
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

//DirLight Vars
glm::vec3 dirLightDirection(0, 0, -1.0f);
glm::vec3 dirLightAmbient(0.2f, 0.2f, 0.2f);
glm::vec3 dirLightDiffuse(0.5f, 0.5f, 0.5f);
glm::vec3 dirLightSpecular(1.0f, 1.0f, 1.0f);
float near_plane = 0.1f;
float far_plane = 50.0f;

//PointLight Vars
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  3.0f,  2.0f),
    glm::vec3(2.3f, 3.0f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  3.0f, -3.0f)
};

glm::vec3 pointLightAmbients[] = {
    glm::vec3(0.2f,  0.2f,  0.2f),
    glm::vec3(0.2f,  0.0f,  0.0f),
    glm::vec3(0.0f,  0.2f,  0.0f),
    glm::vec3(0.0f,  0.0f,  0.2f)
};

glm::vec3 pointLightDiffuses[] = {
    glm::vec3(0.3f,  0.3f,  0.3f),  
    glm::vec3(0.3f,  0.0f,  0.0f),
    glm::vec3(0.0f,  0.3f,  0.0f),
    glm::vec3(0.0f,  0.0f,  0.3f)
};

glm::vec3 pointLightSpeculars[] = {
    glm::vec3(0.5f,  0.5f,  0.5f),  
    glm::vec3(0.5f,  0.0f,  0.0f),
    glm::vec3(0.0f,  0.5f,  0.0f),
    glm::vec3(0.0f,  0.0f,  0.5f)
};

glm::vec3 pointLightHDRColors[] = {
    glm::vec3(5.0f,  5.0f,  5.0f),    // white light with normal intensity
    glm::vec3(10.0f,  0.0f,  0.0f),    // bright red
    glm::vec3(0.0f,  15.0f,  0.0f),    // bright green
    glm::vec3(0.0f,  0.0f,  15.0f)     // bright blue
};

float pointLightConstants[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float pointLightLinears[] = { 0.14f, 0.14f, 0.14f, 0.14f };
float pointLightQuadratics[] = { 0.07f, 0.07f, 0.07f, 0.07f };

std::vector<glm::vec3> objectPositions;

std::vector<float> sphereVertices;
std::vector<unsigned int> sphereIndices;

//Initial values for mouse (center of the screen)
float lastX = windowWidth/2, lastY = windowHeight/2;
bool firstMouse = true;

//Render Mode
RenderMode mRenderMode = DEFAULT;
bool shadows = true;
float exposure = 0.3f;

//Matricies
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 lightProjection = glm::mat4(1.0f);
glm::mat4 lightView = glm::mat4(1.0f);
glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

std::vector<glm::mat4> shadowTransforms;

int main()
{

    generateSphere(1.0f, 36, 18, sphereVertices, sphereIndices);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    /*CallBacks*/
    //Tell GLFW to call framebuffer_size_callback on every window resize by registering it
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //Tell GLFW to call mouse_callback on every mouse input
    glfwSetCursorPosCallback(window, mouse_callback);
    //Tell GLFW to call mouse_callback on every scroll input
    glfwSetScrollCallback(window, scroll_callback);

    //Initialize GLAD before call any OpenGL function
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //Load SkyBox
    unsigned int cubemapTexture = loadCubemap(faces);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);
    //Load Model
    Model ourModel(backpackPath);
    //Generate Model positions
    generateObjectPositions(objectPositions);


    //Enable z-test and face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //Init Shaders
    Shader ourShader = CreateShader("3.3.shader");
    Shader lightShader = CreateShader("lightShader");
    Shader arrowShader = CreateShader("arrowShader");
    Shader screenShader = CreateShader("screenShader");
    Shader skyBoxShader = CreateShader("skyBoxShader");
    Shader reflectiveShader = CreateShader("reflectiveShader");
    Shader normalDisplayShader = CreateShader("normalDisplay", true);  // true for geometry shader
    Shader simpleDepthShader = CreateShader("simpleDepthShader");
    Shader debugQuadShader = CreateShader("debugQuad");
    Shader floorShader = CreateShader("floorShader");
    Shader pointShadowDepthShader = CreateShader("pointShadowDepth", true);  // true for geometry shader
    Shader blurShader = CreateShader("blur");
    Shader gBufferShader = CreateShader("gBuffer");
    Shader deferredShader = CreateShader("deferredShading");

    //GBuffer for Deferred rendering
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec;

    // - position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // - normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // - color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int gBufferAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, gBufferAttachments);

    // Add depth buffer to gBuffer
    unsigned int gDepth;
    glGenRenderbuffers(1, &gDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepth);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Light Buffers
    unsigned int lightVAO, lightVBO, lightEBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glGenBuffers(1, &lightEBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); //positions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); //normals
    

    //Screen quad buffers
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    //skybox buffers
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // plane VAO
    unsigned int planeVAO;
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // configure second post-processing framebuffer
    unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    unsigned int screenTextures[2];
    glGenTextures(2, screenTextures);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, screenTextures[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, screenTextures[i], 0
        );
    }
    unsigned int intermediateAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, intermediateAttachments);
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Shadow Buffer
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Tell openGL to use our shader
    ourShader.use();
    ourShader.setVec3("dirLight.direction", dirLightDirection);
    ourShader.setVec3("dirLight.ambient", dirLightAmbient);
    ourShader.setVec3("dirLight.diffuse", dirLightDiffuse);
    ourShader.setVec3("dirLight.specular", dirLightSpecular);
    floorShader.use();
    floorShader.setVec3("dirLight.direction", dirLightDirection);
    floorShader.setVec3("dirLight.ambient", dirLightAmbient);
    floorShader.setVec3("dirLight.diffuse", dirLightDiffuse);
    floorShader.setVec3("dirLight.specular", dirLightSpecular);

    //load floor texture
    unsigned int woodTexture = loadTexture(floorDiffusePathCstr);

    //Load Point Lights
    int numLights = sizeof(pointLightPositions) / sizeof(pointLightPositions[0]);
    loadPointLightsToShader(ourShader, numLights);
    loadPointLightsToShader(floorShader, numLights);
    loadPointLightsToShader(deferredShader, numLights);

    //Create Arrow
    Arrow arrow = Arrow();

    //Set texture sampler indicies
    ourShader.use();
    ourShader.setInt("shadowMap", 4);
    ourShader.setInt("shadowCubeMap", 5);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);
    screenShader.setInt("bloomBlur", 1);

    skyBoxShader.setInt("skybox", 0);
    reflectiveShader.setInt("skybox", 0);

    debugQuadShader.use();
    debugQuadShader.setInt("screenTexture", 0);

    floorShader.use();
    floorShader.setInt("diffuseTexture", 0);
    floorShader.setInt("shadowMap", 1);
    floorShader.setInt("shadowCubeMap", 2);

    blurShader.use();
    blurShader.setInt("image", 0);

    deferredShader.use();
    deferredShader.setInt("gPosition", 0);
    deferredShader.setInt("gNormal", 1);
    deferredShader.setInt("gAlbedoSpec", 2);
    deferredShader.setInt("shadowMap", 3);
    deferredShader.setVec3("dirLight.direction", dirLightDirection);
    deferredShader.setVec3("dirLight.ambient", dirLightAmbient);
    deferredShader.setVec3("dirLight.diffuse", dirLightDiffuse);
    deferredShader.setVec3("dirLight.specular", dirLightSpecular);

    
    //Render loop
    while (!glfwWindowShouldClose(window))
    {
        //Calculate deltaTime
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float time = static_cast<float>(glfwGetTime());
        float radius = 5.0f;
        dirLightDirection = glm::normalize(glm::vec3(
            sin(time) * radius,  
            4.0f,                
            cos(time) * radius   
        ));
        ourShader.use();
        ourShader.setVec3("dirLight.direction", dirLightDirection);
        floorShader.use();
        floorShader.setVec3("dirLight.direction", dirLightDirection);
        screenShader.use();
        screenShader.setFloat("exposure", exposure);

        //Handle input
        processInput(window);

        //Clear and create a new set of cubmaps for depth cube map
        createDepthCubeMapTransforms(near_plane, far_plane, pointLightPositions[0], shadowTransforms);

        /*Set up shaders*/
        setUpMVP(view, projection, model, lightProjection, lightView, lightSpaceMatrix);
        //ourShader------------------------------------------
        ourShader.passMVP(model, view, projection);
        ourShader.setVec3("cameraPos", mCamera.pos);
        ourShader.setFloat("far_plane", far_plane);
        ourShader.setInt("shadows", shadows);
        //reflectiveShader-----------------------------------
        reflectiveShader.passMVP(model, view, projection);
        reflectiveShader.setVec3("cameraPos", mCamera.pos);
        //normalDisplayShader--------------------------------
        normalDisplayShader.passMVP(model, view, projection);
        normalDisplayShader.setVec3("cameraPos", mCamera.pos);
        normalDisplayShader.setFloat("MAGNITUDE", magnitude);
        //arrowShader
        arrowShader.passMVP(model, view, projection);
        //lightShader-----------------------------------------
        lightShader.passMVP(model, view, projection);
        //skyBoxShader----------------------------------------
        skyBoxShader.use();
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);    // re-enable writing to depth buffer
        glDepthFunc(GL_LESS);    // set depth function back to default

        //simpleDepthShader-----------------------------------
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        simpleDepthShader.setMat4("model", model);
        //floorShader------------------------------------------
        floorShader.use();
        floorShader.setVec3("viewPos", mCamera.pos);
        floorShader.passMVP(model, view, projection);
        floorShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        floorShader.setFloat("far_plane", far_plane);
        floorShader.setInt("shadows", shadows);
        //pointShadowDepthShader--------------------------------
        pointShadowDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            pointShadowDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        pointShadowDepthShader.setFloat("far_plane", far_plane);
        pointShadowDepthShader.setVec3("lightPos", pointLightPositions[0]);
        //deferredShader--------------------------------
        deferredShader.use();
        deferredShader.setVec3("dirLight.direction", dirLightDirection);
        deferredShader.setVec3("dirLight.ambient", dirLightAmbient);
        deferredShader.setVec3("dirLight.diffuse", dirLightDiffuse);
        deferredShader.setVec3("dirLight.specular", dirLightSpecular);
        deferredShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        deferredShader.setInt("shadows", shadows);
        deferredShader.setFloat("far_plane", far_plane);
        deferredShader.setVec3("cameraPos", mCamera.pos);
        

        // ─────────────── Pass 1: render shadow depth map (only depth) ───────────────
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        //*Render scene to depthMap*/
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        // render the loaded model
        glCullFace(GL_BACK);
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        for (unsigned int i = 0; i < objectPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, objectPositions[i]);
            simpleDepthShader.setMat4("model", model);
            ourModel.Draw(simpleDepthShader);
        }
        //render floor
        simpleDepthShader.use();
        renderFloor(simpleDepthShader, planeVAO);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // reset viewport
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        // ─────────────── Pass 2: render scene to gBuffer framebuffer ───────────────
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glEnable(GL_DEPTH_TEST);
        //clear color and depth
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gBufferShader.use();
        gBufferShader.setMat4("view", view);
        gBufferShader.setMat4("projection", projection);

        // render the loaded model
        for (unsigned int i = 0; i < objectPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, objectPositions[i]);
            gBufferShader.setMat4("model", model);
            ourModel.Draw(gBufferShader);
        }

        //render floor
        gBufferShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        gBufferShader.setInt("material.texture_diffuse1", 0);
        gBufferShader.setInt("material.texture_specular1", 0);
        gBufferShader.setInt("material.texture_normal1", 0);
        gBufferShader.setInt("material.texture_roughness1", 0);
        gBufferShader.setInt("hasNormalTexture", 0);
        renderFloor(gBufferShader, planeVAO);

        //render lights
        gBufferShader.use();
        gBufferShader.setInt("hasNormalTexture", 0);
        gBufferShader.setInt("hasDiffuseTexture", 0);
        renderPointLights(gBufferShader, numLights, lightVAO);
        //render arrow
        gBufferShader.use();
        gBufferShader.setInt("hasNormalTexture", 0);
        gBufferShader.setInt("hasDiffuseTexture", 0);
        arrow.Draw(dirLightDirection, gBufferShader, glm::vec3(2, 5, -5), 2.0f, glm::vec3(1, 0, 0));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //─────────────── Pass 3: calculate lighting using the gbuffer's content and render to intermediateFrameBuffer ───────────────
        glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Set clear color to black
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        //render scene
        deferredShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderQuad(quadVAO);


        //render skyBox
        glDepthFunc(GL_GEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        glDepthMask(GL_FALSE);   // disable writing to depth buffer

        // skybox cube
        skyBoxShader.use();
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);    // re-enable writing to depth buffer
        glDepthFunc(GL_LESS);    // set depth function back to default

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ─────────────── Pass 5: blur bright fragments with two-pass Gaussian Blur ───────────────
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 2;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            glViewport(0, 0, windowWidth, windowHeight);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Set clear color to black
            glClear(GL_COLOR_BUFFER_BIT);
            
            blurShader.setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? screenTextures[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad(quadVAO);
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowWidth, windowHeight);

        // ─────────────── Pass 6: Render screen Quad ──────────────
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Set clear color to black
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        //draw screen
        screenShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenTextures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

        renderQuad(quadVAO);

        //─────────────── Pass 7(Optional): Debug screen Quad ──────────────
        if (mRenderMode == DEBUG) {
            // clear all relevant buffers
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            //draw debug quad
            debugQuadShader.use();
            debugQuadShader.setInt("debugMode", 1); // 0: position, 1: normal, 2: albedo
            
            // Display normal buffer
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[0]);
            debugQuadShader.setInt("screenTexture", 0);
            renderQuad(quadVAO);
        }

        // check and call events and swap the buffers
        glfwPollEvents();
        glfwSwapBuffers(window);

        //Update fps display
        float fps = 1.0f / deltaTime;  // FPS = 1/deltaTime
        std::string title = "OpenGL - FPS: " + std::to_string((int)fps);
        glfwSetWindowTitle(window, title.c_str());
        
    }

    //de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteBuffers(1, &lightEBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &intermediateFBO);
    glDeleteBuffers(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &rboDepth);
    glDeleteBuffers(1, &depthMapFBO);
    for (unsigned int fbo : pingpongFBO) {
        glDeleteBuffers(1, &fbo);
    }

    glfwTerminate();

    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    mCamera.ProcessMouseScroll(static_cast<float>(yoffset));
}

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

    //Hide Camera
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = mCamera.sensitivity;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    mCamera.ProcessMousePan(xoffset, yoffset);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    //KeyBoard Control Camera (WASD)
    float cameraSpeedNormalized = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        mCamera.ProcessKeyBoard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mCamera.ProcessKeyBoard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        mCamera.ProcessKeyBoard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        mCamera.ProcessKeyBoard(RIGHT, deltaTime);

    //Toggle render polygon mode
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // wireframe
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // fill

    //Render Mode Control
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
        mRenderMode = REFLECTIVE;
    }
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
        mRenderMode = NORMAL_DISPLAY;
    }
    if(glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
        mRenderMode = DEFAULT;
    }
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
        mRenderMode = DEBUG;
    }
}

void generateSphere(float radius, int sectorCount, int stackCount,
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices)
{
    float x, y, z, xy;                      // vertex position
    float nx, ny, nz;                       // normal
    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;

    for (int i = 0; i <= stackCount; ++i)
    {
        float stackAngle = PI / 2 - i * stackStep;        // from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);                     // r * cos(phi)
        z = radius * sinf(stackAngle);                      // r * sin(phi)

        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = j * sectorStep;             // from 0 to 2pi

            x = xy * cosf(sectorAngle);                     // x = r * cos(phi) * cos(theta)
            y = xy * sinf(sectorAngle);                     // y = r * cos(phi) * sin(theta)
            nx = x / radius;
            ny = y / radius;
            nz = z / radius;

            // vertex: pos + normal
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    // index list
    for (int i = 0; i < stackCount; ++i)
    {
        int k1 = i * (sectorCount + 1);     // beginning of current stack
        int k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void renderPointLights(Shader& lightShader, int numLights, unsigned int& lightVAO) {
    //Render Point Lights
    for (int i = 0; i < numLights; i++)
    {
        //Tell OpenGL to use light shader   
        lightShader.use();
        //Calculate pointLights' Model Matricies
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions[i]);
        model = glm::scale(model, glm::vec3(0.1f));
        //pass model Matrices
        lightShader.setMat4("model", model);
        //set light colors
        lightShader.setVec3("color", pointLightHDRColors[i]);

        //Make lightVAO in Bound and Draw spheres
        glBindVertexArray(lightVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIndices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
}

void renderFloor(Shader& floorShader, unsigned int& planeVAO) {
    floorShader.use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, -1.5f, 0));
    floorShader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void setUpMVP(glm::mat4& view, glm::mat4& projection, glm::mat4& model, glm::mat4& lightProjection, glm::mat4& lightView, glm::mat4& lightSpaceMatrix) {
    //Calculate View Matrix
    view = mCamera.GetViewMat();
    //Calculate Projection Matrix
    projection = glm::perspective(glm::radians(mCamera.fov), (float)windowWidth / (float)windowHeight, mCamera.near, mCamera.far);
    //Calculate Model Matrix
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate to origin
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// unit scale

    // Calculate light space matrix with better coverage
    glm::vec3 lightDir = glm::normalize(dirLightDirection);
    glm::vec3 lightPos = lightDir * 20.0f;  // Move light further away
    
    // orthographic frustum
    float orthoSize = 10.0f; 
    lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);
    
    // Center the light view on the scene
    glm::vec3 center = glm::vec3(0.0f, -0.5f, 0.0f);  // Center of your scene
    lightView = glm::lookAt(lightPos, center, glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void createDepthCubeMapTransforms(float near_plane, float far_plane, glm::vec3 lightPos, std::vector<glm::mat4>& shadowTransforms) {
    shadowTransforms.clear();
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), shadowAspect, near_plane, far_plane);
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
}

void loadPointLightsToShader(Shader& shader, const int numLights) {
    shader.use();
    shader.setInt("NumPointLights", numLights);
    for (int i = 0; i < numLights; i++) {
        std::string varName = "";
        std::string prefix = "pointLights[";
        std::string postfix = "].";
        std::string resultPrefix = prefix + std::to_string(i) + postfix;
        varName = resultPrefix + "position";
        shader.setVec3(varName, pointLightPositions[i]);

        varName = resultPrefix + "ambient";
        shader.setVec3(varName, pointLightAmbients[i]);

        varName = resultPrefix + "diffuse";
        shader.setVec3(varName, pointLightDiffuses[i]);

        varName = resultPrefix + "specular";
        shader.setVec3(varName, pointLightSpeculars[i]);

        varName = resultPrefix + "constant";
        shader.setFloat(varName, pointLightConstants[i]);

        varName = resultPrefix + "linear";
        shader.setFloat(varName, pointLightLinears[i]);

        varName = resultPrefix + "quadratic";
        shader.setFloat(varName, pointLightQuadratics[i]);
    }
}

void renderQuad(const unsigned int quadVAO) {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void generateObjectPositions(std::vector<glm::vec3>& objectPositions) {
    objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));
}