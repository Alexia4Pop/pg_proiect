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
glm::vec3 fireplacePos = glm::vec3(-2.1f, 0.23f, -1.80f);
//camera position: z = -x , y = y, x = -z

const int MAX_CANDLE_PARTICLES = 30;
std::vector<FireParticle> candlePool(MAX_CANDLE_PARTICLES);
glm::vec3 candlePos = glm::vec3(0.12f, 0.84f, -0.32f);

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
GLint modelLoc;

gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 5.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.08f;

bool pressedKeys[1024];
float angle = 0.0f;

gps::Model3D myModel;
gps::Model3D fireParticleModel;

gps::Shader myCustomShader;

// Logic for Fire Lifespan and Movement
void updateFire(float ts) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        auto& p = firePool[i];

        // Let's say the first 20 particles are "Embers"
        if (i < 20) {
            p.life -= ts * 0.5f; // Embers live longer/stay slow
            if (p.life <= 0.0f) {
                // Keep them very close to the fireplacePos
                p.position = fireplacePos + glm::vec3(
                    ((rand() % 100) - 50) / 1000.0f,
                    -0.05f, // Slightly below the flame base
                    ((rand() % 100) - 50) / 1000.0f
                );
                p.velocity = glm::vec3(0.0f, 0.005f, 0.0f); // Very slow drift
                p.life = 1.0f;
            }
        }
        else {
            // THE REST ARE NORMAL FLAMES (Your existing code)
            p.life -= ts * 0.5f;
            if (p.life <= 0.0f) {
                p.position = fireplacePos;
                p.velocity = glm::vec3(((rand() % 100) - 50) / 500.0f, 
                    0.04f + (rand() % 100) / 500.0f, 
                    ((rand() % 100) - 50) / 500.0f);
                p.life = 1.0f;
            }
        }
        p.position += p.velocity * ts;
    }
}

void updateCandleFire(float ts) {
    for (auto& p : candlePool) {
        p.life -= ts * 0.5f; // Fast decay for a tiny flame
        if (p.life <= 0.0f) {
            p.position = candlePos;
            // Very tight spread for a candle wick
            p.velocity = glm::vec3(
                ((rand() % 100) - 50) / 5000.0f,
                0.02f + (rand() % 100) / 5000.0f,
                ((rand() % 100) - 50) / 5000.0f
            );
            p.life = 1.0f;
        }
        p.position += p.velocity * ts;
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
    if (pressedKeys[GLFW_KEY_Q]) angle += 0.08f;
    if (pressedKeys[GLFW_KEY_E]) angle -= 0.08f;
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

    // Lighting for the House
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos"), 1, glm::value_ptr(fireplacePos));
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "viewPos"), 1, glm::value_ptr(myCamera.getCameraPosition()));

    glm::mat4 view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // 1. DRAW HOUSE
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // Normal House Light Color
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isFire"), 0);
    glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f); // Pure white light

    glEnable(GL_DEPTH_TEST);

    myModel.Draw(myCustomShader);

    // 2. DRAW FIRE PARTICLES
    glDepthMask(GL_FALSE); // Particles don't block depth
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for "glow"

    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isFire"), 1); // Tell shader to ignore shadow math

    for (int i = 0; i < MAX_PARTICLES; i++) {
        auto& p = firePool[i];
        if (p.life > 0.0f) {
            glm::mat4 m = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::translate(m, p.position);

            glm::vec3 pColor;
            float currentScale;

            if (i < 20) { // EMBER LOGIC
                currentScale = 0.005f; // Keep them very small like sparks
                // Deep red to dark orange flicker
                float pulse = (sin(glfwGetTime() * 5.0f) + 1.0f) * 0.5f;
                pColor = glm::mix(glm::vec3(0.3f, 0.0f, 0.0f), glm::vec3(0.6f, 0.1f, 0.0f), pulse);
            }
            else { // NORMAL FLAME LOGIC
                currentScale = p.life * 0.035f;
                // 1. Initialize pColor with the flame gradient
                pColor = glm::mix(glm::vec3(0.8f, 0.1f, 0.0f), glm::vec3(1.0f, 0.9f, 0.2f), p.life);

                // 2. Apply the dimming multiplier (0.09f is very dim, perfect for overlap)
                pColor = pColor * 0.09f;
            }

            m = glm::scale(m, glm::vec3(currentScale));
            glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(m));
            glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(pColor));

            fireParticleModel.Draw(myCustomShader);
            //myModel.Draw(myCustomShader);
        }
    }

    // 3. DRAW CANDLE PARTICLES (Inside renderScene, after fireplace loop)
    for (auto& p : candlePool) {
        if (p.life > 0.0f) {
            glm::mat4 m = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::translate(m, p.position);

            // Very small scale for a candle (0.005 instead of 0.035)
            float candleScale = p.life * 0.005f;
            m = glm::scale(m, glm::vec3(candleScale));

            // Slightly bluer base or brighter yellow for a candle
            glm::vec3 cColor = glm::mix(glm::vec3(0.8f, 0.1f, 0.0f), glm::vec3(1.0f, 0.9f, 0.2f), p.life);
            cColor *= 0.1f; // Keep it dim

            glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(m));
            glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(cColor));

            myModel.Draw(myCustomShader);
        }
    }

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, const char* argv[]) {
    srand((unsigned int)time(NULL));
    if (!initOpenGLWindow()) return 1;

    myCustomShader.loadShader("D:/FACULTATE/PG/lab/proiect - Copy/lab6/shaders/shaderStart.vert", "D:/FACULTATE/PG/lab/proiect - Copy/lab6/shaders/shaderStart - Modificat2.frag");
    myModel.LoadModel("D:/FACULTATE/PG/lab/proiect - Copy/lab6/objects/casa3_01.obj", "D:/FACULTATE/PG/lab/proiect - Copy/lab6/textures/");
    fireParticleModel.LoadModel("D:/FACULTATE/PG/lab/proiect - Copy/lab6/objects/casa3_01.obj", "D:/FACULTATE/PG/lab/proiect - Copy/lab6/textures/");


    glm::mat4 projection = glm::perspective(glm::radians(55.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    myCustomShader.useShaderProgram();
    //adaugat
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
    //adaugat
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    for (int i = 0; i < MAX_CANDLE_PARTICLES; i++) {
        candlePool[i].position = candlePos;
        candlePool[i].life = (float)(rand() % 1000) / 1000.0f;
    }

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
        updateCandleFire(ts);
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