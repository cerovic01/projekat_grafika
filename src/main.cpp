#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//komentar

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
// Neki komentar123

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
//void key_callback2(GLFWwindow *window, int key, int scancode, int action, int mod);
unsigned int loadTexture(const char *path);

unsigned int loadCubemap(vector<std::string> skyboxFaces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool lightOn = false;
bool lKeyPressed = false;
bool pointLightOn = true;
bool kKeyPressed = false;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};
struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    DirLight dirLight;
    PointLight pointLight;
    SpotLight spotLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
   // glfwSetKeyCallback(window, key_callback2);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
   // stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    //light
    DirLight& dirLight = programState->dirLight;
    dirLight.direction = glm::vec3(-40.0f, -20.0f, 70.0f);
    dirLight.ambient = glm::vec3(0.04);
    dirLight.diffuse = glm::vec3(0.3, 0.15, 0.0);
    dirLight.specular = glm::vec3(1.0, 1.0, 1.0);


    PointLight& pointLight = programState->pointLight;
    //pointLight.position = glm::vec3(2.6, 1.6, 0.0);
    pointLight.ambient = glm::vec3(0.3);
    pointLight.diffuse = glm::vec3(0.9);
    pointLight.specular = glm::vec3(1.0);
    pointLight.constant = 0.2f;
    pointLight.linear = 0.23f;
    pointLight.quadratic = 0.352f;


    SpotLight& spotLight = programState->spotLight;
    spotLight.position = programState->camera.Position;
    spotLight.direction = programState->camera.Front;
    spotLight.ambient = glm::vec3 (1.0f, 1.0f, 1.0f);
    spotLight.diffuse = glm::vec3 (0.7f, 0.7f, 0.7f);
    spotLight.specular = glm::vec3 (0.5f, 0.5f, 0.5f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.05f;
    spotLight.quadratic = 0.012f;
    spotLight.cutOff = glm::cos(glm::radians(10.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(13.0f));

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // build and compile shaders
    // -------------------------
    Shader ourShader(FileSystem::getPath("resources/shaders/2.model_lighting.vs").c_str(), FileSystem::getPath("resources/shaders/2.model_lighting.fs").c_str());
    Shader skyboxShader(FileSystem::getPath("resources/shaders/skybox.vs").c_str(), FileSystem::getPath("resources/shaders/skybox.fs").c_str());
    Shader blendingShader(FileSystem::getPath("resources/shaders/blending.vs").c_str(), FileSystem::getPath("resources/shaders/blending.fs").c_str());
    Shader moonShader(FileSystem::getPath("resources/shaders/moon.vs").c_str(), FileSystem::getPath("resources/shaders/moon.fs").c_str());

    //cube
    float vertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    float skyboxVertices[] = {
            //positions
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

    // floor coordinates
    float floorVertices[] = {
            // positions          // normals          // texture coords
            0.5f,  0.5f,  0.0001f,  0.0f, 0.0f, -1.0f,  50.0f,  50.0f,  // top right
            0.5f, -0.5f,  0.0001f,  0.0f, 0.0f, -1.0f,  50.0f,  0.0f,  // bottom right
            -0.5f, -0.5f,  0.0001f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,  // bottom left
            -0.5f,  0.5f,  0.0001f,  0.0f, 0.0f, -1.0f,  0.0f,  50.0f   // top left
    };

    unsigned int floorIndices[] = {
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
    };
    // floor coordinates
    float putVertices[] = {
            // positions          // normals          // texture coords
            0.031f, 0.05f,  0.0f,  0.0f, 0.0f, -1.0f,  8.0f,  3.0f,  // top right
            0.031f, -0.2f,  0.0f,  0.0f, 0.0f, -1.0f,  8.0f,  9.0f,  // bottom right
            -0.013f, -0.2f,  0.0f,  0.0f, 0.0f, -1.0f,  9.0f,  9.0f,  // bottom left
            -0.013f,  0.05f,  0.0f,  0.0f, 0.0f, -1.0f,  9.0f,  3.0f   // top left
    };

    unsigned int putIndices[] = {
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
    };

    float transparentVertices[] = {
            // positions         // texture coords
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    //cube VAO
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //skybox VAO
    unsigned int skyboxVBO, skyboxVAO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Floor VAO
    unsigned int floorVAO, floorVBO, floorEBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);


    //put
    unsigned int putVAO, putVBO, putEBO;
    glGenVertexArrays(1, &putVAO);
    glGenBuffers(1, &putVBO);
    glGenBuffers(1, &putEBO);

    glBindVertexArray(putVAO);
    glBindBuffer(GL_ARRAY_BUFFER, putVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(putVertices), putVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, putEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(putIndices), putIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    //transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    //glBindVertexArray(0);

    //load box textures
    unsigned int diffuseMap  = loadTexture(FileSystem::getPath("resources/textures/box.png").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/box_specular.png").c_str());

    ourShader.use();
    ourShader.setInt("material.texture_diffuse1", 0);
    ourShader.setInt("material.texture_specular1", 1);

    //load skybox textures
    vector<std::string> skyboxFaces{
            FileSystem::getPath("resources/textures/right1.jpg"),
            FileSystem::getPath("resources/textures/left1.jpg"),
            FileSystem::getPath("resources/textures/top1.jpg"),
            FileSystem::getPath("resources/textures/bottom1.jpg"),
            FileSystem::getPath("resources/textures/front1.jpg"),
            FileSystem::getPath("resources/textures/back1.jpg")
    };

    unsigned int cubemapTexture = loadCubemap(skyboxFaces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    //load put textures
    unsigned int putDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/stone_box.jpg").c_str());
    unsigned int putSpecularMap = loadTexture(FileSystem::getPath("resources/textures/stone_specular.png").c_str());

    //load floor textures
    unsigned int floorDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/trava.jpg").c_str());
    unsigned int floorSpecularMap = loadTexture(FileSystem::getPath("resources/textures/travaspec.png").c_str());

    //load grass texture
    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/grass.png").c_str());
    unsigned int transparentTexture1 = loadTexture(FileSystem::getPath("resources/textures/grass1.png").c_str());

    blendingShader.use();
    blendingShader.setInt("texture1", 0);

    vector<glm::vec3> grassPos {
        //glm::vec3 (-1.5f, 0.0f, -2.0f),
        //glm::vec3 (1.5f, 0.0f, -2.0f),
        //glm::vec3 (2.5f, 0.0f, -2.0f),
            glm::vec3(2.3, 0, 0.15),
            glm::vec3(-1.2, 0, 2.15),
            glm::vec3(2.3, 0, -4.15),
            glm::vec3(-1.2, 0, -2.15),
            glm::vec3(2.3, 0, -8.15),
            glm::vec3(-1.2, 0, -6.15),
            glm::vec3(2.3, 0, -12.15),
            glm::vec3(-1.2, 0, -10.15),
            glm::vec3(2.3, 0, -16.15),
            glm::vec3(-1.2, 0, -14.15),

            glm::vec3(2.3, 0, -0.15),
            glm::vec3(-1.2, 0, 1.85),
            glm::vec3(2.3, 0, -3.85),
            glm::vec3(-1.2, 0, -1.85),
            glm::vec3(2.3, 0, -7.85),
            glm::vec3(-1.2, 0, -5.85),
            glm::vec3(2.3, 0, -11.85),
            glm::vec3(-1.2, 0, -9.85),
            glm::vec3(2.3, 0, -15.85),
            glm::vec3(-1.2, 0, -13.85)


    };

    vector<glm::vec3> lampPos {
        glm::vec3(3, -0.335, 0),
        glm::vec3(-0.5, -0.335, 2),
        glm::vec3(3, -0.335, -4),
        glm::vec3(-0.5, -0.335, -2),
        glm::vec3(3, -0.335, -8),
        glm::vec3(-0.5, -0.335, -6),
        glm::vec3(3, -0.335, -12),
        glm::vec3(-0.5, -0.335, -10),
        glm::vec3(3, -0.335, 0-16),
        glm::vec3(-0.5, -0.335, -14)
    };
    vector<glm::vec3> lightPos {
            glm::vec3(2.6, 1.6, 0),
            glm::vec3(-0.9, 1.6, 2),
            glm::vec3(2.6, 1.6, -4),
            glm::vec3(-0.9, 1.6, -2),
            glm::vec3(2.6, 1.6, -8),
            glm::vec3(-0.9, 1.6, -6),
            glm::vec3(2.6, 1.6, -12),
            glm::vec3(-0.9, 1.6, -10),
            glm::vec3(2.6, 1.6, 0-16),
            glm::vec3(-0.9, 1.6, -14)
    };



    // load models
    Model lamp(FileSystem::getPath("resources/objects/lamp/streetlamp.obj"));
    lamp.SetShaderTextureNamePrefix("material.");

    Model moon(FileSystem::getPath("resources/objects/moon/planet.obj"));
    moon.SetShaderTextureNamePrefix("material.");


    Model house(FileSystem::getPath("resources/objects/house/farmhouse_obj.obj"));
    house.SetShaderTextureNamePrefix("material.");

    Model lampion(FileSystem::getPath("resources/objects/lampion/light.obj"));
    lampion.SetShaderTextureNamePrefix("material.");



    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // don't forget to enable shader before setting uniforms
        ourShader.use();

        ourShader.setVec3("dirLight.direction", dirLight.direction);
        ourShader.setVec3("dirLight.ambient", dirLight.ambient);
        ourShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        ourShader.setVec3("dirLight.specular", dirLight.specular);


       // pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));

       ourShader.setInt("pointLightOn", pointLightOn);
       ourShader.setVec3("pointLight1.position", lightPos[0]);
       ourShader.setVec3("pointLight1.ambient", pointLight.ambient);
       ourShader.setVec3("pointLight1.diffuse", pointLight.diffuse);
       ourShader.setVec3("pointLight1.specular", pointLight.specular);
       ourShader.setFloat("pointLight1.constant", pointLight.constant);
       ourShader.setFloat("pointLight1.linear", pointLight.linear);
       ourShader.setFloat("pointLight1.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight2.position", lightPos[1]);
        ourShader.setVec3("pointLight2.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight2.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight2.specular", pointLight.specular);
        ourShader.setFloat("pointLight2.constant", pointLight.constant);
        ourShader.setFloat("pointLight2.linear", pointLight.linear);
        ourShader.setFloat("pointLight2.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight3.position", lightPos[2]);
        ourShader.setVec3("pointLight3.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight3.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight3.specular", pointLight.specular);
        ourShader.setFloat("pointLight3.constant", pointLight.constant);
        ourShader.setFloat("pointLight3.linear", pointLight.linear);
        ourShader.setFloat("pointLight3.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight4.position", lightPos[3]);
        ourShader.setVec3("pointLight4.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight4.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight4.specular", pointLight.specular);
        ourShader.setFloat("pointLight4.constant", pointLight.constant);
        ourShader.setFloat("pointLight4.linear", pointLight.linear);
        ourShader.setFloat("pointLight4.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight5.position", lightPos[4]);
        ourShader.setVec3("pointLight5.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight5.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight5.specular", pointLight.specular);
        ourShader.setFloat("pointLight5.constant", pointLight.constant);
        ourShader.setFloat("pointLight5.linear", pointLight.linear);
        ourShader.setFloat("pointLight5.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight6.position", lightPos[5]);
        ourShader.setVec3("pointLight6.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight6.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight6.specular", pointLight.specular);
        ourShader.setFloat("pointLight6.constant", pointLight.constant);
        ourShader.setFloat("pointLight6.linear", pointLight.linear);
        ourShader.setFloat("pointLight6.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight7.position", lightPos[6]);
        ourShader.setVec3("pointLight7.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight7.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight7.specular", pointLight.specular);
        ourShader.setFloat("pointLight7.constant", pointLight.constant);
        ourShader.setFloat("pointLight7.linear", pointLight.linear);
        ourShader.setFloat("pointLight7.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight8.position", lightPos[7]);
        ourShader.setVec3("pointLight8.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight8.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight8.specular", pointLight.specular);
        ourShader.setFloat("pointLight8.constant", pointLight.constant);
        ourShader.setFloat("pointLight8.linear", pointLight.linear);
        ourShader.setFloat("pointLight8.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight9.position", lightPos[8]);
        ourShader.setVec3("pointLight9.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight9.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight9.specular", pointLight.specular);
        ourShader.setFloat("pointLight9.constant", pointLight.constant);
        ourShader.setFloat("pointLight9.linear", pointLight.linear);
        ourShader.setFloat("pointLight9.quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLight10.position", lightPos[9]);
        ourShader.setVec3("pointLight10.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight10.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight10.specular", pointLight.specular);
        ourShader.setFloat("pointLight10.constant", pointLight.constant);
        ourShader.setFloat("pointLight10.linear", pointLight.linear);
        ourShader.setFloat("pointLight10.quadratic", pointLight.quadratic);

        ourShader.setVec3("lampion.ambient", glm::vec3(0.3, 0.2, 0.0));
        ourShader.setVec3("lampion.diffuse", pointLight.diffuse);
        ourShader.setVec3("lampion.specular", glm::vec3(0.6));
        ourShader.setFloat("lampion.constant", pointLight.constant);
        ourShader.setFloat("lampion.linear", pointLight.linear);
        ourShader.setFloat("lampion.quadratic", pointLight.quadratic);


        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        ourShader.setInt("lightOn", lightOn);
        ourShader.setVec3("spotLight.position", programState->camera.Position);
        ourShader.setVec3("spotLight.direction", programState->camera.Front);
        ourShader.setVec3("spotLight.ambient", spotLight.ambient);
        ourShader.setVec3("spotLight.diffuse", spotLight.diffuse);
        ourShader.setVec3("spotLight.specular", spotLight.specular);
        ourShader.setFloat("spotLight.constant", spotLight.constant);
        ourShader.setFloat("spotLight.linear", spotLight.linear);
        ourShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        ourShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        ourShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);



        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


      /*  if (programState->ImGuiEnabled)
            DrawImGui(programState);*/

        //lampion
        glm::mat4 model = glm::mat4 (1.0f);
        model = glm::translate(model, glm::vec3(0.2, 1.15, -18.0));
        model = glm::scale(model, glm::vec3(0.6f));
        model = glm::translate(model, glm::vec3(0.0f, 1.32f, 0.0f));
        model = glm::rotate(model, glm::radians((float)(25.0 * sin(1.0 + 2*glfwGetTime()))), glm::vec3(1.0f, 0.0f, (sin(glfwGetTime())+1)/2));
        model = glm::translate(model, glm::vec3(0.0f, -1.32f, 0.0f));

        glm::vec3 pointLightPositions = glm::vec3(model * glm::vec4(0.0f, 0.2f, 0.0f, 1.0f));
        ourShader.setVec3("lampion.position", pointLightPositions);

        ourShader.setMat4("model", model);
        lampion.Draw(ourShader);


        //lamp
        for (unsigned int i = 0; i < lampPos.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lampPos[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            ourShader.setMat4("model", model);
            lamp.Draw(ourShader);
        }


        //house
        model = glm::mat4 (1.0f);
        model = glm::translate(model, glm::vec3(1.0, -0.335, -21.0));
        model = rotate(model, glm::radians(180.0f),glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(0.2f));
        ourShader.setMat4("model", model);
        house.Draw(ourShader);

        //gull

        model = glm::mat4 (1.0f);
        model = glm::translate(model, glm::vec3(-0.9, 0.8, 2));
        model = rotate(model, (float)glfwGetTime()*4,glm::vec3(0.0, 0.001, 0.0));

     //   model = glm::rotate(model, glm::radians((float)(25.0 * sin(1.0 + 2*glfwGetTime()))), glm::vec3(1.0f, 0.0f, (sin(glfwGetTime())+1)/2));
         model = glm::translate(model, glm::vec3(-0.45, 0.8, 0.00001));
        model = glm::scale(model, glm::vec3(0.003f));
        ourShader.setMat4("model", model);
        moon.Draw(ourShader);

        //moon
        moonShader.use();
        moonShader.setMat4("projection", projection);
        moonShader.setMat4("view", view);
        model = glm::mat4 (1.0f);
        model = glm::translate(model, glm::vec3(40.0f, 20.0f, -70.0f));
        moonShader.setMat4("model", model);
        moon.Draw(moonShader);

        //draw cube
        ourShader.use();
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.67f));
        ourShader.setMat4("model", model);

        //diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        //specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        ourShader.setFloat("material.shininess", 200.0f);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // floor setup
        //ourShader.setVec3("pointLight.specular", 0.01f, 0.03f, 0.01f);
        ourShader.setVec3("dirLight.specular", 0.01f, 0.03f, 0.01f);
        ourShader.setFloat("material.shininess", 10.0f);

        // world transformation
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.335, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f));
        ourShader.setMat4("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorDiffuseMap);

        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorSpecularMap);

        // render floor
        glBindVertexArray(floorVAO);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glDisable(GL_CULL_FACE);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, putDiffuseMap);

        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, putSpecularMap);

        // render floor
        glBindVertexArray(putVAO);
        glEnable(GL_CULL_FACE);     // floor won't be visible if looked from bellow
        glCullFace(GL_BACK);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glDisable(GL_CULL_FACE);


        //grass
        blendingShader.use();
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);

        projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();

        blendingShader.setMat4("projection", projection);
        blendingShader.setMat4("view", view);

        for (unsigned int i = 0; i < grassPos.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, grassPos[i]);
            model = glm::scale(model, glm::vec3(0.65f));
            blendingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }


      //  glBindTexture(GL_TEXTURE_2D, transparentTexture1);
     //   model = glm::translate(model,  glm::vec3 (4.6f, 0.0f, -0.3f));
       // blendingShader.setMat4("model", model);
      //  glDrawArrays(GL_TRIANGLES, 0, 6);

        //draw skybox
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view",view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &skyboxVBO);


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);

    //spot light
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !lKeyPressed){
        lightOn = !lightOn;
        lKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
    {
        lKeyPressed = false;
    }
    //point light
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !kKeyPressed){
        pointLightOn = !pointLightOn;
        kKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE)
    {
        kKeyPressed = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->backpackPosition);
        ImGui::DragFloat("Backpack scale", &programState->backpackScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
              glEnable(GL_MULTISAMPLE);
    }
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
           glDisable(GL_MULTISAMPLE);
          }
}

//void key_callback2(GLFWwindow *window, int key, int scancode, int action, int mod) {
 //   if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
 //       glEnable(GL_MULTISAMPLE);
 //   }
 //   if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
   //     glDisable(GL_MULTISAMPLE);
  //  }
//}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
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