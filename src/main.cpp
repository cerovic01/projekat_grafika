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
unsigned int loadTexture1(char const * path, bool gammaCorrection);

unsigned int loadCubemap(vector<std::string> skyboxFaces);

void renderQuad();
void renderQuad2();
void renderQuad3();
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool lightOn = false;
bool lKeyPressed = false;
bool pointLightOn = true;
bool kKeyPressed = false;
bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 0.2f;
bool bloom = true;
bool bloomKeyPressed = false;
bool antialiasing=true; //  DODAOO
float heightScale = 0.1;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
void setShader(Shader ourShader, DirLight dirLight, PointLight pointLight, SpotLight spotLight, vector<glm::vec3> lightPos);

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
    dirLight.diffuse = glm::vec3(0.3, 0.1, 0.0);
    dirLight.specular = glm::vec3(0.4, 0.3, 0.2);


    PointLight& pointLight = programState->pointLight;
    //pointLight.position = glm::vec3(2.6, 1.6, 0.0);
    pointLight.ambient = glm::vec3(1.5);
    pointLight.diffuse = glm::vec3(0.9);
    pointLight.specular = glm::vec3(1.0);
    pointLight.constant = 0.2f;
    pointLight.linear = 0.23f;
    pointLight.quadratic = 0.352f;


    SpotLight& spotLight = programState->spotLight;
    spotLight.position = programState->camera.Position;
    spotLight.direction = programState->camera.Front;
    spotLight.ambient = glm::vec3 (0.5f);
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

    Shader lightShader (FileSystem::getPath("resources/shaders/light.vs").c_str(), FileSystem::getPath("resources/shaders/light.fs").c_str());
    Shader shaderBlur(FileSystem::getPath("resources/shaders/blur.vs").c_str(), FileSystem::getPath("resources/shaders/blur.fs").c_str());
    Shader shaderBloomFinal(FileSystem::getPath("resources/shaders/bloomfinal.vs").c_str(), FileSystem::getPath("resources/shaders/bloomfinal.fs").c_str());
    Shader Normalshader(FileSystem::getPath("resources/shaders/normal_mapping.vs").c_str(), FileSystem::getPath("resources/shaders/normal_mapping.fs").c_str());
    Shader Parshader(FileSystem::getPath("resources/shaders/5.1.parallax_mapping.vs").c_str(), FileSystem::getPath("resources/shaders/5.1.parallax_mapping.fs").c_str());
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

    // configure (floating point) framebuffers
    // ---------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    // shader configuration
    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);


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
    unsigned int putDiffuseMap = loadTexture1(FileSystem::getPath("resources/textures/stone_box.jpg").c_str(), true);
    unsigned int putSpecularMap = loadTexture(FileSystem::getPath("resources/textures/stone_specular.png").c_str());

    //load floor textures
    unsigned int floorDiffuseMap = loadTexture1(FileSystem::getPath("resources/textures/trava.jpg").c_str(), true);
    unsigned int floorSpecularMap = loadTexture(FileSystem::getPath("resources/textures/travaspec.png").c_str());

    //load grass texture
    unsigned int transparentTexture = loadTexture1(FileSystem::getPath("resources/textures/grass.png").c_str(), true);
    unsigned int transparentTexture1 = loadTexture(FileSystem::getPath("resources/textures/grass1.png").c_str());

    blendingShader.use();
    blendingShader.setInt("texture1", 0);




    vector<glm::vec3> grassPos {
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



    unsigned int NormaldiffuseMap = loadTexture(FileSystem::getPath("resources/textures/brickwall.jpg").c_str());
    unsigned int normal1Map = loadTexture(FileSystem::getPath("resources/textures/brickwall_normal.jpg").c_str());
    Normalshader.use();
    Normalshader.setInt("NormaldiffuseMap", 0);
    Normalshader.setInt("normal1Map", 1);

    unsigned int PardiffuseMap = loadTexture(FileSystem::getPath("resources/textures/bricks2.jpg").c_str());
    unsigned int ParnormalMap  = loadTexture(FileSystem::getPath("resources/textures/bricks2_normal.jpg").c_str());
    unsigned int ParheightMap  = loadTexture(FileSystem::getPath("resources/textures/bricks2_disp.jpg").c_str());


    // shader configuration
    // --------------------
    Parshader.use();
    Parshader.setInt("PardiffuseMap", 0);
    Parshader.setInt("ParnormalMap", 1);
    Parshader.setInt("PardepthMap", 2);


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


        setShader(ourShader, dirLight, pointLight, spotLight, lightPos);
        setShader(lightShader, dirLight, pointLight, spotLight, lightPos);

        // 1. render scene into floating point framebuffer
        // -----------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /*  if (programState->ImGuiEnabled)
            DrawImGui(programState);*/


        //lampion
        lightShader.use();
        glm::mat4 model = glm::mat4 (1.0f);
        model = glm::translate(model, glm::vec3(0.2, 1.15, -18.0));
        model = glm::scale(model, glm::vec3(0.6f));
        model = glm::translate(model, glm::vec3(0.0f, 1.32f, 0.0f));
        model = glm::rotate(model, glm::radians((float)(25.0 * sin(1.0 + 2*currentFrame))), glm::vec3(1.0f, 0.0f, (sin(currentFrame)+1)/2));
        model = glm::translate(model, glm::vec3(0.0f, -1.32f, 0.0f));

        glm::vec3 pointLightPositions = glm::vec3(model * glm::vec4(0.0f, 0.2f, 0.0f, 1.0f));
        lightShader.setVec3("lampion.position", pointLightPositions);

        lightShader.setMat4("model", model);
        lampion.Draw(lightShader);

        ourShader.use();
        ourShader.setVec3("lampion.position", pointLightPositions);

        //lamp
        lightShader.use();
        for (unsigned int i = 0; i < lampPos.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lampPos[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightShader.setMat4("model", model);
            lamp.Draw(lightShader);
        }


        //gull
        lightShader.use();
        model = glm::mat4 (1.0f);
        model = glm::translate(model, glm::vec3(-0.9, 0.8, 2));
        model = rotate(model, (float)glfwGetTime()*4,glm::vec3(0.0, 0.001, 0.0));

     //   model = glm::rotate(model, glm::radians((float)(25.0 * sin(1.0 + 2*glfwGetTime()))), glm::vec3(1.0f, 0.0f, (sin(glfwGetTime())+1)/2));
         model = glm::translate(model, glm::vec3(-0.45, 0.8, 0.00001));
        model = glm::scale(model, glm::vec3(0.003f));
        lightShader.setMat4("model", model);
        moon.Draw(lightShader);


        //moon
        moonShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        moonShader.setMat4("projection", projection);
        moonShader.setMat4("view", view);
        model = glm::mat4 (1.0f);
        model = glm::translate(model, glm::vec3(40.0f, 20.0f, -70.0f));
        moonShader.setMat4("model", model);
        moon.Draw(moonShader);



        //house
        ourShader.use();
        model = glm::mat4 (1.0f);
        model = glm::translate(model, glm::vec3(1.0, -0.335, -21.0));
        model = rotate(model, glm::radians(180.0f),glm::vec3(0.0, 1.0, 0.0));
        model = glm::scale(model, glm::vec3(0.2f));
        ourShader.setMat4("model", model);
        house.Draw(ourShader);

        //zid

        Normalshader.use();
        setShader(Normalshader,dirLight,pointLight,spotLight,lightPos);
      //  Normalshader.setMat4("projection", projection);
      //  Normalshader.setMat4("view", view);
        // render normal-mapped quad
        //model = glm::translate(model, glm::vec3(1.0, -0.335, -210.0));
        model = glm::mat4(1.0f);

        Normalshader.setMat4("model", model);
        Normalshader.setVec3("viewPos", camera.Position);
        Normalshader.setVec3("lightPos", 0.5f, 1.0f, 0.3f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, NormaldiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal1Map);
        renderQuad2();

        //zid2

        Parshader.use();
        Parshader.setMat4("projection", projection);
        Parshader.setMat4("view", view);
        // render parallax-mapped quad
        glm::mat4 model1 = glm::mat4(1.0f);
        Parshader.setMat4("model", model1);
        Parshader.setVec3("viewPos", camera.Position);
        Parshader.setVec3("lightPos", 0.5f, 1.0f, 0.3f);
        Parshader.setFloat("heightScale", heightScale); // adjust with Q and E keys
        std::cout << heightScale << std::endl;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, PardiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ParnormalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ParheightMap);
        renderQuad3();


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

        ourShader.setFloat("material.shininess", 800.0f);

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



        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        shaderBlur.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloomFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        shaderBloomFinal.setInt("hdr", hdr);
        shaderBloomFinal.setInt("bloom", bloom);
        shaderBloomFinal.setFloat("exposure", exposure);
        renderQuad();

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
unsigned int quad2VAO = 0;
unsigned int quad2VBO;
void renderQuad2()
{
    if (quad2VAO == 0)
    {
        // positions
        glm::vec3 pos1(2.5f,  0.7f, -19.5f);
        glm::vec3 pos2(2.5f, -0.50f, -19.5f);
        glm::vec3 pos3( 5.0f, -0.50f, -19.5f);
        glm::vec3 pos4( 5.0f,  0.7f, -19.5f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.5f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.5f, 0.0f);
        glm::vec2 uv4(1.5f, 1.5f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);
        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);
        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent2);

        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);

        float quadVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quad2VAO);
        glGenBuffers(1, &quad2VBO);
        glBindVertexArray(quad2VAO);
        glBindBuffer(GL_ARRAY_BUFFER, quad2VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quad2VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int quad3VAO = 0;
unsigned int quad3VBO;
void renderQuad3()
{
    if (quad3VAO == 0)
    {
        // positions
        glm::vec3 pos1(-3.50f,  1.4f, -19.5f);
        glm::vec3 pos2(-3.50f, -0.50f, -19.5f);
        glm::vec3 pos3( 0.5f, -0.50f, -19.5f);
        glm::vec3 pos4( 0.5f,  1.4f, -19.5f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.5f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.5f, 0.0f);
        glm::vec2 uv4(1.5f, 1.5f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);
        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);
        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent2);

        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);

        float quadVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quad3VAO);
        glGenBuffers(1, &quad3VBO);
        glBindVertexArray(quad3VAO);
        glBindBuffer(GL_ARRAY_BUFFER, quad3VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quad3VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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


    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed)
    {
        hdr = !hdr;
        hdrKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        hdrKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.02f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        exposure += 0.02f;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
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
   // if(key == GLFW_KEY_M && action == GLFW_PRESS){
    //    if (antialiasing) {
     //       glDisable(GL_MULTISAMPLE);
     //       antialiasing=!antialiasing;
      //  }
     //   else {
      //      glEnable(GL_MULTISAMPLE);
      //      antialiasing = !antialiasing;
      //  }

  //  }
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

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

unsigned int loadTexture1(char const * path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
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

void setShader(Shader ourShader, DirLight dirLight, PointLight pointLight, SpotLight spotLight, vector<glm::vec3> lightPos){
    ourShader.use();

    ourShader.setVec3("dirLight.direction", dirLight.direction);
    ourShader.setVec3("dirLight.ambient", dirLight.ambient);
    ourShader.setVec3("dirLight.diffuse", dirLight.diffuse);
    ourShader.setVec3("dirLight.specular", dirLight.specular);

    ourShader.setInt("pointLightOn", pointLightOn);

    for(unsigned int i=1; i<=10; ++i){
        ourShader.setVec3("pointLight" + std::to_string(i) + ".position", lightPos[i-1]);
        ourShader.setVec3("pointLight" + std::to_string(i) + ".ambient", pointLight.ambient);
        ourShader.setVec3("pointLight" + std::to_string(i) + ".diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight" + std::to_string(i) + ".specular", pointLight.specular);
        ourShader.setFloat("pointLight" + std::to_string(i) + ".constant", pointLight.constant);
        ourShader.setFloat("pointLight" + std::to_string(i) + ".linear", pointLight.linear);
        ourShader.setFloat("pointLight" + std::to_string(i) + ".quadratic", pointLight.quadratic);
        if(i==4 || i==7){ //broken lamp
            double r = ((double) rand() / (RAND_MAX));
            if(r<0.5)
                r=0;
            else r=1;
            ourShader.setVec3("pointLight" + std::to_string(i) + ".ambient", glm::vec3((sin(3* glfwGetTime())+1)/6.67 * r));
        }
    }


    ourShader.setVec3("lampion.ambient", glm::vec3(2.0, 1.5, 0.1));
    ourShader.setVec3("lampion.diffuse", pointLight.diffuse);
    ourShader.setVec3("lampion.specular", glm::vec3(0.7, 0.6, 0.5));
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
}