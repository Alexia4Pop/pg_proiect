#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <ctime>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

// --- FIRE PARTICLE SYSTEM ---
struct FireParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life; // 1.0 (new) to 0.0 (dead)
};

const int MAX_PARTICLES = 100;
std::vector<FireParticle> firePool(MAX_PARTICLES);
//glm::vec3 fireplacePos = glm::vec3(0.23927f, 1.8422f, 0.036482f);
//Camera Position: X: -1.95, Y: 0.30, Z: 2.14
glm::vec3 fireplacePos = glm::vec3(-14.82f, 8.56f, 0.72f);
//camera position: x=x y=z , z=-y

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
GLint modelLoc;

gps::Camera myCamera(glm::vec3(-15.0f, 7.6f, -0.17f), glm::vec3(0.0f, 5.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.08f;

bool pressedKeys[1024];
float angle = 0.0f;

gps::Model3D myModel;
gps::Model3D fireParticleModel;
gps::Shader myCustomShader;

// Logic for Fire Lifespan and Movement
void updateFire(float ts) {
    for (auto& p : firePool) {
        if (p.life <= 0.0f) {
            p.position = fireplacePos;
            // Add more randomness to X and Z to make the fire "wider"
            float randX = ((rand() % 100) - 50) / 500.0f;
            float randZ = ((rand() % 100) - 50) / 500.0f;
            float randY = (rand() % 100) / 500.0f; // Upward speed

            p.velocity = glm::vec3(randX, 0.1f + randY, randZ);
            p.life = 1.0f; // Reset life
        }
        p.position += p.velocity * ts;
        p.life -= ts * 0.7f; // Decrease life slowly for longer flames
    }
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) pressedKeys[key] = true;
        else if (action == GLFW_RELEASE) pressedKeys[key] = false;
    }
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_Q]) angle += 0.8f;
    if (pressedKeys[GLFW_KEY_E]) angle -= 0.8f;
    if (pressedKeys[GLFW_KEY_W]) myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    if (pressedKeys[GLFW_KEY_S]) myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    if (pressedKeys[GLFW_KEY_A]) myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    if (pressedKeys[GLFW_KEY_D]) myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    if (pressedKeys[GLFW_KEY_SPACE]) myCamera.move(gps::MOVE_UP, cameraSpeed);
    if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) myCamera.move(gps::MOVE_DOWN, cameraSpeed);
}

bool initOpenGLWindow() {
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "Fireplace Animation", NULL, NULL);
    if (!glWindow) return false;

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwMakeContextCurrent(glWindow);

    glewExperimental = GL_TRUE;
    glewInit();
    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
    return true;
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, retina_width, retina_height);
    myCustomShader.useShaderProgram();

    // 1. CALCULATE WORLD POSITIONS (Account for House Rotation)
    // We create a rotation matrix based on your 'angle' variable
    glm::mat4 sceneRot = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Rotate the Fireplace Light Position
    glm::vec3 rotatedFireplacePos = glm::vec3(sceneRot * glm::vec4(fireplacePos, 1.0f));

    // Rotate the Street Lamp Positions
    glm::vec3 rawLampPos[] = {
        glm::vec3(-0.84f, 8.64f, -7.03f),// The coordinates you found
        glm::vec3(-3.20f, 7.97f, -0.15f),    // Lamp 2
        glm::vec3(-16.93f, 9.19f, -19.04f),     // Lamp 3
        glm::vec3(-18.70f, 9.01f, 13.44f),
        glm::vec3(-18.70f, 9.01f, -13.44f),
        glm::vec3(-24.76f, 10.55f, -37.17f),
        glm::vec3(-34.12f, 10.55f, -14.16f),
        glm::vec3(-37.59f, 10.71f, 21.34f),
        glm::vec3(-50.93f, 10.71f, 26.91f),
        glm::vec3(-70.59f, 12.53f, 29.19f)
    };

    glm::vec3 finalLamps[10];
    for (int i = 0; i < 10; i++) {
        finalLamps[i] = glm::vec3(sceneRot * glm::vec4(rawLampPos[i], 1.0f));
    }

    // 2. SEND LIGHTING UNIFORMS TO SHADER
    // Fireplace Uniforms
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos"), 1, glm::value_ptr(rotatedFireplacePos));

    // Lamp Uniforms (Positions and Color)
    GLuint lampPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lampPositions");
    glUniform3fv(lampPosLoc, 10, glm::value_ptr(finalLamps[0]));
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "lampColor"), 1.0f, 0.95f, 0.7f); // Warm White

    // Camera/View Uniforms
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "viewPos"), 1, glm::value_ptr(myCamera.getCameraPosition()));
    glm::mat4 view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));


    // 3. DRAW THE HOUSE
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isFire"), 0); // Lighting ON
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    model = sceneRot; // The house uses the rotation matrix
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    myModel.Draw(myCustomShader);


    // 4. DRAW FIRE PARTICLES
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isFire"), 1); // Lighting OFF (Emissive)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow
    glDepthMask(GL_FALSE);            // Particles don't hide things behind them

    for (auto& p : firePool) {
        if (p.life > 0.0f) {
            // Apply same rotation as house, then move to particle position
            glm::mat4 m = sceneRot;
            m = glm::translate(m, p.position);

            // Scale based on life (makes them shrink as they die)
            float currentScale = p.life * 0.02f;
            m = glm::scale(m, glm::vec3(currentScale));

            glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(m));

            // Calculate color: Dim it by 0.1f to prevent "blinding white" overlap
            glm::vec3 pColor = glm::mix(glm::vec3(1.0f, 0.2f, 0.0f), glm::vec3(1.0f, 0.9f, 0.3f), p.life);
            pColor *= 0.1f;

            glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(pColor));

            fireParticleModel.Draw(myCustomShader);
        }
    }
    // The Sun (Directional Light)
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "sunDir"), 1.0f, 1.0f, 0.5f);
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "sunColor"), 0.2f, 0.15f, 0.1f);

    // RESET STATE
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

int main(int argc, const char* argv[]) {
    srand((unsigned int)time(NULL));
    if (!initOpenGLWindow()) return 1;

    myCustomShader.loadShader("D:/FACULTATE/PG/lab/proiect - Copy/lab6/shaders/shaderStart - Copy.vert", "D:/FACULTATE/PG/lab/proiect - Copy/lab6/shaders/shaderStart - Modificat2 - Copy - Copy.frag");
    myModel.LoadModel("D:/FACULTATE/PG/lab/proiect - Copy/lab6/objects/proiect_copie_doamneajuta_join_cu_casa2.obj", "D:/FACULTATE/PG/lab/proiect - Copy/lab6/textures/");
    fireParticleModel.LoadModel("D:/FACULTATE/PG/lab/proiect - Copy/lab6/objects/casa3_01.obj", "D:/FACULTATE/PG/lab/proiect - Copy/lab6/textures/");

    glm::mat4 projection = glm::perspective(glm::radians(55.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    myCustomShader.useShaderProgram();
    //adaugat
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
    //adaugat
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    float lastFrame = 0.0f;
    // Inside main, before the while loop:
    for (int i = 0; i < MAX_PARTICLES; i++) {
        firePool[i].position = fireplacePos;
        firePool[i].life = (float)(rand() % 1000) / 1000.0f; // Random life between 0 and 1
    }
    while (!glfwWindowShouldClose(glWindow)) {
        float currentFrame = (float)glfwGetTime();
        float ts = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processMovement();
        updateFire(ts);
        renderScene();

        glm::vec3 camPos = myCamera.getCameraPosition();
        printf("Camera Position: X: %.2f, Y: %.2f, Z: %.2f\n", camPos.x, camPos.y, camPos.z);

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }
    glfwTerminate();
    return 0;
}
//