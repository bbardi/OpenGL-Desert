#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>                  //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp>   //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp>         //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 modelDesert;
glm::mat4 modelCasa;
glm::mat4 modelHeli;
glm::mat4 modelHeliBlades;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 modelWater;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
GLuint pointLightLoc;
GLuint pointLightColorLoc;
GLuint fogLoc;
GLuint clipPlaneLoc;
GLuint reflectTex;
GLuint refractTex;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 1.0f, 3.0f),
    glm::vec3(0.0f, 10.0f, 10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D desert;
gps::Model3D casa;
gps::Model3D heli;
gps::Model3D heliBlades;

// shaders
gps::Shader myBasicShader;
gps::Shader skyBoxShader;
gps::Shader waterShader;

// skybox
gps::SkyBox mySkyBox;

// animation parameters
float deltaMov = 0;
float deltaAngle = 0;
float heliBladeAngle = 0;
float movementSpeed = 0.01f;
float angularSpeed = 10;
enum HELIANIM
{
    FORWARD,
    ROTATING,
    LEFT,
    ROTATING2
};
bool right = false, backward = false;
float heliAngle = 0;
HELIANIM animation = FORWARD;
glm::vec3 HeliPos(0.0f, 0.0f, 0.0f);
void updateDelta(double elapsedSeconds)
{
    deltaMov = movementSpeed * elapsedSeconds * 500;
    deltaAngle = angularSpeed * elapsedSeconds * 300;
}
double lastTimeStamp = glfwGetTime();

// yaw pitch
GLdouble yaw = 0, pitch = 0;

// fog
GLboolean fog;
// Second Light Source
glm::vec3 lightColor2(0.5f,0.5f,0.0f);
glm::vec3 pointLight(0.0f,1.0f,0.0f);

// FBO for water
GLuint FBO[2];
// Texture for Water
GLuint WaterTex[2];
// Depth FBO
GLuint DepthFBO[2];
// Depth Texture
GLuint DepthTex[2];

glm::vec4 ReflectclipPlane(0,1,0,0.1f);
glm::vec4 RefractclipPlane(0,-1,0,-0.1f);
glm::vec4 NoclipPlane(0,-1,0,10000);

GLuint WaterVAO,WaterVBO;
GLfloat WaterData[]{
    -1, -1, 
    -1, 1, 
    1, -1, 
    1, -1, 
    -1, 1, 
    1, 1
};
GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow *window, int width, int height)
{
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    WindowDimensions newDimensions;
    //get the new dimensions
    glfwGetFramebufferSize(window, &newDimensions.width, &newDimensions.height);
    myWindow.setWindowDimensions(newDimensions);
    //remake the projection matrix and send it to the shader
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                                  0.1f, 1000.0f);
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    //set the viewport to the new dimensions
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            pressedKeys[key] = true;
            if (key == GLFW_KEY_F)
            {
                fog = !fog;
                myBasicShader.useShaderProgram();
                glUniform1i(fogLoc,fog);
                skyBoxShader.useShaderProgram();
                GLuint skyboxFogLoc = glGetUniformLocation(skyBoxShader.shaderProgram, "fog");;
                glUniform1i(skyboxFogLoc,fog);
            }
        }
        else if (action == GLFW_RELEASE)
        {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    GLfloat sensitivity = 10;
    glfwGetCursorPos(myWindow.getWindow(), &yaw, &pitch);

    if (pitch / sensitivity > 87)
    {
        pitch = sensitivity * 86.99;
    }
    if (pitch / sensitivity < -87)
    {
        pitch = sensitivity * -86.99;
    }

    myCamera.rotate(-pitch / sensitivity, yaw / sensitivity);
    myBasicShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void processMovement()
{
    if (pressedKeys[GLFW_KEY_W])
    {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * modelDesert));
    }

    if (pressedKeys[GLFW_KEY_S])
    {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * modelDesert));
    }

    if (pressedKeys[GLFW_KEY_A])
    {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * modelDesert));
    }

    if (pressedKeys[GLFW_KEY_D])
    {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * modelDesert));
    }
    
}

void initOpenGLWindow()
{
    myWindow.Create(800, 600, "Desert Scene");
}

void setWindowCallbacks()
{
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState()
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS);    // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE);  // cull face
    glCullFace(GL_BACK);     // cull back face
    glFrontFace(GL_CCW);     // GL_CCW for counter clock-wise
    glEnable(GL_CLIP_DISTANCE0);
}

void initModels()
{
    desert.LoadModel("models/desert2/desert.obj");
    casa.LoadModel("models/casa/casa.obj");
    heli.LoadModel("models/Heli/heli_no_blades.obj");
    heliBlades.LoadModel("models/Heli/blades.obj");
}

void initShaders()
{
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyBoxShader.loadShader(
        "shaders/skyboxShader.vert",
        "shaders/skyboxShader.frag");
    waterShader.loadShader(
        "shaders/water.vert",
        "shaders/water.frag");
}

void initSkyBox()
{
    std::vector<const GLchar *> faces;
    faces.push_back("textures/skybox/right.tga");
    faces.push_back("textures/skybox/left.tga");
    faces.push_back("textures/skybox/top.tga");
    faces.push_back("textures/skybox/bottom.tga");
    faces.push_back("textures/skybox/back.tga");
    faces.push_back("textures/skybox/front.tga");
    mySkyBox.Load(faces);
}

void initUniforms()
{
    myBasicShader.useShaderProgram();

    // create model matrix for desert
    modelDesert = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
    modelCasa = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    modelCasa = glm::translate(modelCasa, glm::vec3(125.0f, 0.0f, 0.0f));
    modelHeli = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 100.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    modelWater = glm::scale(glm::mat4(1.0f),glm::vec3(20.0f,20.0f,20.0f));
    modelWater = glm::translate(modelWater,glm::vec3(0.0f,-0.1f,0.0f));
    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for desert
    normalMatrix = glm::mat3(glm::inverseTranspose(view * modelDesert));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                                  0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 0.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    //set point light color
    pointLightLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight");
    pointLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColor");
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(lightColor2));
    glUniform3fv(pointLightLoc, 1, glm::value_ptr(pointLight));
    //getting fog info and sending it to shader
    fogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fog");
    fog = false;
    glUniform1i(fogLoc,fog);
    clipPlaneLoc = glGetUniformLocation(myBasicShader.shaderProgram,"clipPlane");
    glUniform4fv(clipPlaneLoc,1,glm::value_ptr(NoclipPlane));
    skyBoxShader.useShaderProgram();
    GLuint skyboxFogLoc = glGetUniformLocation(skyBoxShader.shaderProgram, "fog");;
    glUniform1i(skyboxFogLoc,fog);
    waterShader.useShaderProgram();
    reflectTex = glGetUniformLocation(waterShader.shaderProgram,"reflection");
    refractTex = glGetUniformLocation(waterShader.shaderProgram,"refraction");
}

void initFBO(){
    //generate FBO and Texure
    glGenFramebuffers(2,FBO);
    glGenRenderbuffers(2,DepthFBO);
    glGenTextures(2,WaterTex);
    glGenTextures(2,DepthTex);
    //Create 1st texture and framebuff
    glBindFramebuffer(GL_FRAMEBUFFER,FBO[0]);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindTexture(GL_TEXTURE_2D,WaterTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, WaterTex[0], 0);
    //Create 1st depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, DepthFBO[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,2048,2048);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,DepthFBO[0]);
    //Create 1st depth texture
    /*glBindTexture(GL_TEXTURE_2D,DepthTex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthTex[0], 0);*/
    //Create 2nd texture and framebuff
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER,FBO[1]);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBindTexture(GL_TEXTURE_2D,WaterTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, WaterTex[1], 0);
    //Create 2nd depth texture
    glBindTexture(GL_TEXTURE_2D,DepthTex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, DepthTex[1], 0);
    //reset
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glBindTexture(GL_TEXTURE_2D,0);
    glBindRenderbuffer(GL_RENDERBUFFER,0);
}

void initWater(){
    glGenVertexArrays(1,&WaterVAO);
    glBindVertexArray(WaterVAO);

    glGenBuffers(1, &WaterVBO);
    glBindBuffer(GL_ARRAY_BUFFER, WaterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WaterData), WaterData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void renderWater(gps::Shader shader){
    shader.useShaderProgram();
    GLuint projLoc = glGetUniformLocation(shader.shaderProgram, "projection");
    GLuint viewLoc = glGetUniformLocation(shader.shaderProgram, "view");
    GLuint modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelWater));
    glUniform1i(reflectTex,0);
    glUniform1i(refractTex,1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,WaterTex[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,WaterTex[1]);
    glBindVertexArray(WaterVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void renderDesert(gps::Shader shader)
{
    shader.useShaderProgram();

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDesert));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * modelDesert));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw terrain
    desert.Draw(shader);
}

void renderHouse(gps::Shader shader)
{
    shader.useShaderProgram();

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCasa));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * modelCasa));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw terrain
    casa.Draw(shader);
}

void renderHelicopter(gps::Shader shader)
{
    shader.useShaderProgram();

    double currentTimeStamp = glfwGetTime();
    updateDelta(currentTimeStamp - lastTimeStamp);
    lastTimeStamp = currentTimeStamp;

    switch (animation)
    {
    case LEFT:
    {
        if (right)
        {
            modelHeli = glm::translate(modelHeli, glm::vec3(0.0f, 0.0f, -deltaMov));
            HeliPos += glm::vec3(0.0f, 0.0f, deltaMov);
            if (HeliPos.z >= 0)
            {
                animation = ROTATING2;
                heliAngle = 0;
                HeliPos = glm::vec3(0.0f);
                right = !right;
            }
        }
        else
        {
            modelHeli = glm::translate(modelHeli, glm::vec3(0.0f, 0.0f, -deltaMov));
            HeliPos += glm::vec3(0.0f, 0.0f, -deltaMov);
            if (HeliPos.z <= -movementSpeed * 6000)
            {
                animation = ROTATING2;
                heliAngle = 0;
                right = !right;
            }
        }
        break;
    }
    case ROTATING:
    {
        modelHeli = glm::rotate(modelHeli, glm::radians(5*deltaMov), glm::vec3(0.0f, 1.0f, 0.0f));
        heliAngle += 5*deltaMov;
        if (heliAngle >= 90.0f)
        {
            animation = LEFT;
            heliAngle = 0;
        }
        break;
    }
    case FORWARD:
    {
        if (backward)
        {
            modelHeli = glm::translate(modelHeli, glm::vec3(0.0f, 0.0f, -deltaMov));
            HeliPos += glm::vec3(0.0f, 0.0f, deltaMov);
            if (HeliPos.z >= 0)
            {
                animation = ROTATING;
                heliAngle = 0;
                backward = !backward;
                HeliPos = glm::vec3(-movementSpeed * 6000);
            }
        }
        else
        {
            modelHeli = glm::translate(modelHeli, glm::vec3(0.0f, 0.0f, -deltaMov));
            HeliPos += glm::vec3(0.0f, 0.0f, -deltaMov);
            if (HeliPos.z <= -movementSpeed * 6000)
            {
                animation = ROTATING;
                heliAngle = 0;
                backward = !backward;
                HeliPos = glm::vec3(0.0f);
            }
        }
        break;
    }
    case ROTATING2:
    {
        modelHeli = glm::rotate(modelHeli, glm::radians(deltaMov*5), glm::vec3(0.0f, 1.0f, 0.0f));
        heliAngle += 5*deltaMov;
        if (heliAngle > 90.0f)
        {
            animation = FORWARD;
            heliAngle = 0;
        }
        break;
    }
    }
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelHeli));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * modelHeli));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw helicopter(bladeless)
    heli.Draw(shader);
    heliBladeAngle += deltaAngle;
    modelHeliBlades = glm::rotate(modelHeli, glm::radians(heliBladeAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelHeliBlades));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * modelHeliBlades));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    // draw helicopter blades
    heliBlades.Draw(shader);
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reflection Render Pass
    glm::mat4 TexProjection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 0.5f);
    gps::Camera reflectCam = myCamera;
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer(GL_FRAMEBUFFER,FBO[0]);
    glViewport(0,0,2048,2048);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    myBasicShader.useShaderProgram();
    float dist = 2*(reflectCam.cameraPosition.y + 0.1f);
    reflectCam.move(gps::MOVE_DOWN,dist);
    reflectCam.rotate(-pitch,yaw);
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc,1,GL_FALSE,glm::value_ptr(reflectCam.getViewMatrix()));
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc,1,GL_FALSE,glm::value_ptr(TexProjection));
    glUniform4fv(clipPlaneLoc,1,glm::value_ptr(ReflectclipPlane));
    renderDesert(myBasicShader);
    renderHouse(myBasicShader);
    renderHelicopter(myBasicShader);
    glUniformMatrix4fv(viewLoc,1,GL_FALSE,glm::value_ptr(view));
    mySkyBox.Draw(skyBoxShader, reflectCam.getViewMatrix(), projection);

 
    // Refraction Render Pass
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer(GL_FRAMEBUFFER,FBO[1]);
    glViewport(0,0,2048,2048);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    myBasicShader.useShaderProgram();
    glUniform4fv(clipPlaneLoc,1,glm::value_ptr(RefractclipPlane));
    renderDesert(myBasicShader);
    renderHouse(myBasicShader);
    renderHelicopter(myBasicShader);
    mySkyBox.Draw(skyBoxShader, view, projection);

    // render the terrain
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    myBasicShader.useShaderProgram();
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc,1,GL_FALSE,glm::value_ptr(projection));
    glUniform4fv(clipPlaneLoc,1,glm::value_ptr(NoclipPlane));
    renderDesert(myBasicShader);
    renderHouse(myBasicShader);
    renderHelicopter(myBasicShader);
    // render the skybox
    mySkyBox.Draw(skyBoxShader, view, projection);
    // render the water
    renderWater(waterShader);
    glCheckError();
}

void cleanup()
{
    myWindow.Delete();
    //cleanup code for your own data
    glDeleteFramebuffers(2,FBO);
    glDeleteTextures(2,WaterTex);
    glDeleteRenderbuffers(2,DepthFBO);
    glDeleteTextures(2,DepthTex);
    glDeleteVertexArrays(1,&WaterVAO);
    glDeleteBuffers(1,&WaterVBO);
}

int main(int argc, const char *argv[])
{
    try
    {
        initOpenGLWindow();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    initSkyBox();
    initFBO();
    initWater();
    setWindowCallbacks();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow()))
    {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
