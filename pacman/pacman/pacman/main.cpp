#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <string>
#include <iostream>
#include <string.h>
#include <vector>
 
#include "pacman.h"

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <shader.h>
#include <arcball.h>
#include <Model.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// FUNCTION PROTOTYPES
GLFWwindow *glAllInit();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void shaderSetting(Shader *shader);
void gameSetting(void);


// GLOBAL VARIABLES

// Source and Data directories
string sourceDirStr = "/Users/mcl/Desktop/공부/Computer Graphics/pacman/pacman/pacman";
string modelDirStr = "/Users/mcl/Desktop/공부/Computer Graphics/pacman/data";

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
GLFWwindow *mainWindow = NULL;
Shader *animShader = NULL;
Shader *lightShader = NULL;

// for lighting
glm::vec3 lightSize(0.1f, 0.1f, 0.1f);
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.f,  0.f,  15.f),
    glm::vec3( 0.f, 0.f, -15.f),
    glm::vec3( 15.f, 0.f, 0.f),
    glm::vec3( -15.f, 0.f, 0.f),
    glm::vec3( 0.f, 15.f, 0.f),
    glm::vec3( 0.f, -15.f, 0.f)
};



// camera
glm::vec3 cameras[6] = {
    glm::vec3(0.f, 0.f, 30.f),
    glm::vec3(30.f, 0.f, 0.f),
    glm::vec3(0.f, -30.f, 0.f),
    glm::vec3(0.f, 0.f, -30.f),
    glm::vec3(-30.f, 0.f, 0.f),
    glm::vec3(0.f, 30.f, 0.f)
};

glm::vec3 ups[6] = {
    glm::vec3(0.f, 1.f, 0.f),
    glm::vec3(0.f, 1.f, 0.f),
    glm::vec3(1.f, 0.f, 0.f),
    glm::vec3(1.f, 0.f, 0.f),
    glm::vec3(0.f, 0.f, -1.f),
    glm::vec3(0.f, 0.f, -1.f)
};

// game data
PacMan *game = nullptr;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float CycleTime = 0.0f;
 
int main()
{
    mainWindow = glAllInit();
    // set game data
    gameSetting();

    // build and compile shaders
    // -------------------------
    string vs = sourceDirStr + "/modelLoading.vs";
    string fs = sourceDirStr + "/modelLoading.fs";
    lightShader = new Shader(vs.c_str(), fs.c_str());
    shaderSetting(lightShader);

    vs = sourceDirStr + "/skel_anim.vs";
    fs = sourceDirStr + "/modelLoading.fs";
    animShader = new Shader(vs.c_str(), fs.c_str());
    shaderSetting(animShader);

    // load models
    
    std::string yonseiManDirStr = modelDirStr + "/yonseiman/yonseiman.gltf";
    Model yonseiMan(yonseiManDirStr);
    Animation yonseiAnim(yonseiManDirStr, &yonseiMan);
    Animator yonseiAnimator(&yonseiAnim);
    
    std::string koreaManDirStr = modelDirStr + "/koreaman/koreaman.gltf";
    Model koreaMan(koreaManDirStr);
    Animation koreaAnim(koreaManDirStr, &koreaMan);
    Animator koreaAnimator(&koreaAnim);
    
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(mainWindow))
    {
        if(game->currMap == ENDING){
            float wait = glfwGetTime();
            while(wait + 15.f > glfwGetTime());
            glfwSetWindowShouldClose(mainWindow, true);
        }
        
        // per-frame time logic
        // --------------------
        int ret;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
 
        float inputCycle = glfwGetTime();
        // input
        // -----
        processInput(mainWindow);
        if(inputCycle - CycleTime >= 1.0f){
            ret = game->cycle();
            CycleTime = inputCycle;
        }
        
        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameras[game->currMap],
                                     glm::vec3(0.f, 0.f, 0.f), ups[game->currMap]);
 
        lightShader->use();
        lightShader->setVec3("viewPos", cameras[game->currMap]);
        lightShader->setMat4("projection", projection);
        lightShader->setMat4("view", view);

        game->render(lightShader);                          // render cube maps
        game->maps[game->currMap].render(lightShader);      // render coins

        animShader->use();
        animShader->setVec3("viewPos", cameras[game->currMap]);
        animShader->setMat4("projection", projection);
        animShader->setMat4("view", view);
 
        yonseiAnimator.UpdateAnimation(deltaTime);

        auto transforms = yonseiAnimator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            animShader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // render the loaded model
        glm::mat4 model(1.0f); 
    
        
        CubeSide mapSide = game->maps[game->currMap].mapSide;
        float xPos = game->maps[game->currMap].yonseiMan.xPos;
        float yPos = game->maps[game->currMap].yonseiMan.yPos;
        Input dir = game->maps[game->currMap].yonseiMan.dir;
        
        float x = 7.5f - xPos;
        float y = -7.5f + yPos;
        
        if(mapSide == Top){
            // do nothing (ending page)
        }
        else if(mapSide == Bottom){
            model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.0f, 1.f, 0.f));
            model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 0.f, 1.f));
        }
        else if(mapSide == Right){
            model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
            model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.f, 0.f, 1.f));
        }
        else if(mapSide == Left){
            model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
        }
        else if(mapSide == Front){
            model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
        }
        else if(mapSide == Behind){
            model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
            model = glm::rotate(model, glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
        }

        
        
        model = glm::translate(model, glm::vec3(y, 0.f, -x));
        model = glm::translate(model, glm::vec3(0.0f, 7.5f, 0.0f)); // translate it down so it's at the center of the scene
        
        if(dir == R) {
            model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
        }
        if(dir == U) {
            model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
        }
        if(dir == L) {
            model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.f, 1.f, 0.f));
        }
        
        if(game->maps[game->currMap].getAction() != NOTHING){
            model = glm::translate(model, glm::vec3(0.f, 0.f,  -1 + inputCycle - CycleTime));
        }
        
        model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));    // it's a bit too big for our scene, so scale it down
 
        animShader->setMat4("model", model);
        yonseiMan.Draw(*animShader);
        
        koreaAnimator.UpdateAnimation(deltaTime);
        transforms = koreaAnimator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            animShader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        for(auto koreaman = game->maps[game->currMap].koreaMans.begin();
            koreaman != game->maps[game->currMap].koreaMans.end(); koreaman++){

            // render the loaded model
            model = glm::mat4(1.0f);
        
            
            CubeSide mapSide = game->maps[game->currMap].mapSide;
            float xPos = (*koreaman).xPos;
            float yPos = (*koreaman).yPos;
            Input dir = (*koreaman).direction;
            
            float x = 7.5f - xPos;
            float y = -7.5f + yPos;
                
            if(mapSide == Top){
                // do nothing (ending page)
            }
            else if(mapSide == Bottom){
                model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.0f, 1.f, 0.f));
                model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 0.f, 1.f));
            }
            else if(mapSide == Right){
                model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
                model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.f, 0.f, 1.f));
            }
            else if(mapSide == Left){
                model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
            }
            else if(mapSide == Front){
                model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
            }
            else if(mapSide == Behind){
                model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
                model = glm::rotate(model, glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
            }
 
            
            
            model = glm::translate(model, glm::vec3(y, 0.f, -x));
            model = glm::translate(model, glm::vec3(0.0f, 7.5f, 0.0f)); // translate it down so it's at the center of the scene
            
            if(dir == R) {
                model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
            }
            if(dir == U) {
                model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
            }
            if(dir == L) {
                model = glm::rotate(model, glm::radians(270.f), glm::vec3(0.f, 1.f, 0.f));
            }
            
            if((*koreaman).wallCheck()){
                model = glm::translate(model, glm::vec3(0.f, 0.f, -inputCycle + CycleTime));
                model = glm::rotate(model,glm::radians((inputCycle - CycleTime) * 180.f),
                                    glm::vec3(0.f, 1.f, 0.f));
            }
            else model = glm::translate(model, glm::vec3(0.f, 0.f, inputCycle - CycleTime));
            
            model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));    // it's a bit too big for our scene, so scale it down
     
            animShader->setMat4("model", model);
            koreaMan.Draw(*animShader);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(mainWindow);
        glfwPollEvents();
        
        if(ret == 1){
            float wait = glfwGetTime();
            glfwSetWindowShouldClose(mainWindow, true);
            while(wait + 5.f > glfwGetTime());
        }
        
        else if(ret == 2){
            float wait = glfwGetTime();
            while(wait + 0.5f > glfwGetTime());
        }
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    
    delete game->cube;
    delete game;
    delete animShader;
    delete lightShader;
    return 0;
}

GLFWwindow *glAllInit()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Skeletal Animation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }
    
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);
    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    
    return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        game->maps[game->currMap].setAction(L);
    }
    else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        game->maps[game->currMap].setAction(R);
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        game->maps[game->currMap].setAction(D);
    }
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        game->maps[game->currMap].setAction(U);
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------


void shaderSetting(Shader *shader){
    // lighting setting
    shader->use();
     
    shader->setFloat("shininess", 32);
    
//     directional light
    shader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    
    for(int i = 0; i < 6; i++){
        std::string str = "pointLights[" + std::to_string(i) + "].";

        shader->setVec3(str + "position", pointLightPositions[i]);
        shader->setVec3(str + "ambient", 0.5f, 0.5f, 0.5f);
        shader->setVec3(str + "diffuse", 1.f, 1.f, 1.f);
        shader->setVec3(str + "specular", 1.2f, 1.2f, 1.2f);
        shader->setFloat(str + "constant",1.0f);
        shader->setFloat(str + "linear", 0.2); // 0.09
        shader->setFloat(str + "quadratic", 0.032);
    }
    // lighting setting end
}

void gameSetting(void){
    char mapData[6][MAP_SIZE * MAP_SIZE];

    memcpy(mapData[0],
    "1111111111111111140000000004140111101111101111011410140010000101101010001020010110101110100001011000000010000001101110111110010110101000101001011010141110100111100011140000000111101111111110011000001000001001101111141110110110000000100000011111111111111111",
    MAP_SIZE * MAP_SIZE);
    memcpy(mapData[1], 
    "1111111111111111140000011000004111100001100001111001000110001001100010011001000110000101101000011000010110100001100000000000000110000101101000011040010110100401100001011010000110001001100100011001000110001001111000011000011114000001100000211111111111111111",
    MAP_SIZE * MAP_SIZE);
    memcpy(mapData[2], 
    "1111111111111111140001000010004110010001100010011001111111111001100000011000000111111101101111111400010110100041100000011000000110000001100000011400010110100041111111011011111110004101100100011010110110010101101011011001010110000001100001211111111111111111",
    MAP_SIZE * MAP_SIZE);
    memcpy(mapData[3], 
    "1111111111111111100000000000004110111110111111011011111111111101101100000004110110110111101011011011000001101101100111010100110110114000011010011011111111101101101111101110110114000000000011011111111111111101111111101111110112000000000000011111111111111111",
    MAP_SIZE * MAP_SIZE);
    memcpy(mapData[4], 
    "1111111111111111100000000000000110111111111110011412000000401001111110111000100110001010000100011010111400010001101011101010000110101000010000011010110010000001101400010000000110100110000000011011100000000001100000000000000110000000000000011111111111111111",
    MAP_SIZE * MAP_SIZE);
    memcpy(mapData[5], 
    "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    MAP_SIZE * MAP_SIZE);

    std::vector<Korea> baseball, football, rugby, basketball, hockey, ending;
    basketball.push_back(Korea(1, 2, 1, 2, 1, 10, L));
    basketball.push_back(Korea(4, 1, 4, 1, 10, 1, U));
    basketball.push_back(Korea(4, 5, 4, 5, 4, 7, L));
    basketball.push_back(Korea(1, 14, 1, 14, 8, 14, U));
    basketball.push_back(Korea(8, 5, 8, 5, 8, 7, L));
    basketball.push_back(Korea(8, 9, 8, 9, 10, 9, U));
    basketball.push_back(Korea(12, 5, 12, 5, 12, 1, R));
    basketball.push_back(Korea(14, 9, 14, 9, 14, 14, L)); 

    for(auto it = basketball.begin(); it != basketball.end(); it++)
        mapData[BASKETBALL][(*it).xPos * MAP_SIZE + (*it).yPos] = '3';

    football.push_back(Korea(7, 1, 7, 1, 7, 14, L));
    football.push_back(Korea(1, 6, 1, 6, 14, 6, U));
    football.push_back(Korea(1, 9, 1, 9, 14, 9, U));

    for(auto it = football.begin(); it != football.end(); it++)
        mapData[FOOTBALL][(*it).xPos * MAP_SIZE + (*it).yPos] = '3';

    hockey.push_back(Korea(1, 2, 1, 2, 4, 2, U));
    hockey.push_back(Korea(1, 13, 1, 13, 4, 13, U));
    hockey.push_back(Korea(7, 1, 7, 1, 7, 6, L));
    hockey.push_back(Korea(8, 1, 8, 1, 8, 6, L));
    hockey.push_back(Korea(7, 9, 7, 9, 7, 14, L));
    hockey.push_back(Korea(8, 9, 8, 9, 8, 14, L));
    hockey.push_back(Korea(11, 3, 11, 3, 14, 3, U));
    hockey.push_back(Korea(11, 10, 11, 10, 14, 10, U));

    for(auto it = hockey.begin(); it != hockey.end(); it++)
        mapData[HOCKEY][(*it).xPos * MAP_SIZE + (*it).yPos] = '3';

    rugby.push_back(Korea(1, 2, 1, 2, 1, 13, L));
    rugby.push_back(Korea(2, 1, 2, 1, 10, 1, U));
    rugby.push_back(Korea(4, 5, 4, 5, 4, 10, L));
    rugby.push_back(Korea(6, 6, 6, 6, 8, 6, U));
    rugby.push_back(Korea(5, 11, 5, 11, 10, 11, U));
    rugby.push_back(Korea(2, 14, 2, 14, 13, 14, U));
    rugby.push_back(Korea(11, 2, 11, 2, 11, 10, L));
    rugby.push_back(Korea(14, 2, 14, 2, 14, 13, L));

    for(auto it = rugby.begin(); it != rugby.end(); it++)
        mapData[RUGBY][(*it).xPos * MAP_SIZE + (*it).yPos] = '3';

    baseball.push_back(Korea(3, 5, 3, 5, 5, 5, U));
    baseball.push_back(Korea(3, 9, 3, 9, 7, 9, U));
    baseball.push_back(Korea(8, 5, 8, 5, 8, 8, L));
    baseball.push_back(Korea(7, 11, 7, 11, 7, 14, L));
    baseball.push_back(Korea(7, 11, 7, 11, 14, 11, U));
    baseball.push_back(Korea(11, 7, 11, 7, 11, 14, L));
    baseball.push_back(Korea(11, 7, 11, 7, 14, 7, U));

    for(auto it = baseball.begin(); it != baseball.end(); it++)
        mapData[BASEBALL][(*it).xPos * MAP_SIZE + (*it).yPos] = '3';

    game = new PacMan(mapData, basketball, football, hockey, rugby, baseball, ending, modelDirStr);
}
