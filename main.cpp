//
//  main.cpp
//  OpenGL_Shader_Example_step1
//
//  Created by CGIS on 02/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
	#define GLFW_INCLUDE_GLCOREARB
#else
	#define GLEW_STATIC
	#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>//core glm functionality
#include <glm/gtc/matrix_transform.hpp>//glm extension for generating common transformation matrices
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
GLint modelLoc;

gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 5.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.05f;

bool pressedKeys[1024];
float angle = 0.0f;

gps::Model3D myModel;
gps::Shader myCustomShader;

GLuint verticesVBO;
GLuint verticesEBO;
GLuint objectVAO;
GLuint texture;

//vertex position and UV coordinates
//vertex position and UV coordinates 
GLfloat vertexData[] = {
	//  primul triunghi 
	-5.0f, 0.0f, 0.0f,    0.0f, 0.0f,
	5.0f,0.0f, 0.0f,    1.0f, 0.0f,
	0.0f, 8.0f, 0.0f,    0.5f, 1.0f,
	//  al doilea triunghi 
	0.1f, 8.0f, 0.0f,    0.0f, 0.0f,
	5.1f, 0.0f, 0.0f,    1.0f, 0.0f,
	10.1f, 8.0f, 0.0f,    0.5f, 1.0f
};
GLuint vertexIndices[] = {
 0,1,2,
 3,4,5
};

void windowResizeCallback(GLFWwindow* window, int width, int height) {

	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {

		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)	{

		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

}

void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
	// Move UP (Space Bar)
	if (pressedKeys[GLFW_KEY_SPACE]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}

	// Move DOWN (Left Control)
	if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}
}

bool initOpenGLWindow() {

	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	// for multisampling/antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Texturing", NULL, NULL);

	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    //glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

GLuint ReadTextureFromFile(const char* file_name) {

	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);

	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
		);
	}

	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {

		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;

		for (int col = 0; col < width_in_bytes; col++) {

			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		//schimbat din SRGB
		GL_RGBA, //GL_SRGB, //GL_SRGB,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

float delta = 0;
/*void renderScene()
{
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, retina_width, retina_height);

	//initialize the model matrix
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	myCustomShader.useShaderProgram();

	//initialize the view matrix
	glm::mat4 view = myCamera.getViewMatrix();
	//send matrix data to shader
	GLint viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	delta += 0.001;
	model = glm::translate(model, glm::vec3(delta, 0, 0));
	//create rotation matrix
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//send matrix data to vertex shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//glActiveTexture(GL_TEXTURE0);
	//glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	//glBindTexture(GL_TEXTURE_2D, texture);
	myModel.Draw(myCustomShader);

	/*glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "diffuseTexture"), 0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(objectVAO); 
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
}*/

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, retina_width, retina_height);

	// 1. Initialize shader
	myCustomShader.useShaderProgram();

	// --- LIGHTING LOGIC START ---
	// 1. Get the light position (Replace these numbers with the ones from Blender's N-panel)
	glm::vec3 candlePos = glm::vec3(0.16607f, 1.15f, -0.23762f);
	GLint lightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(candlePos));

	// 2. Set Light Color (Warm candle orange: R=1.0, G=0.7, B=0.3)
	glm::vec3 lightColor = glm::vec3(1.0f, 0.7f, 0.3f);
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));

	// 3. Set View Position (Required for specular/shiny highlights)
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "viewPos"), 1, glm::value_ptr(myCamera.getCameraPosition()));
	// --- LIGHTING LOGIC END ---

	// 2. Send View Matrix
	glm::mat4 view = myCamera.getViewMatrix();
	GLint viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// 3. Model Matrix (Positioning the plant)
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	// Stop the plant from sliding away! (Optional)
	// You currently have 'delta += 0.001' which makes the plant move to the right forever.
	// If you want it to stay still, comment out the next two lines:
	// delta += 0.001; 
	// model = glm::translate(model, glm::vec3(delta, 0, 0)); 

	// Rotate it if needed (Blender exports sometimes come in rotated)
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

	// Send Model Matrix
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// 4. Draw the Model
	// The Model3D class automatically looks inside the FBX, finds the texture, 
	// and passes it to the shader.
	//ADAUGAT
	glDisable(GL_CULL_FACE);
	//ADAUGAT

	myModel.Draw(myCustomShader);

	//ADAUGAT
	glEnable(GL_CULL_FACE);
	//ADAUGAT
}

void loadTriangleData() {

	glGenVertexArrays(1, &objectVAO);

	glBindVertexArray(objectVAO);

	glGenBuffers(1, &verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glGenBuffers(1, &verticesEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, verticesEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexIndices), vertexIndices, GL_STATIC_DRAW);

	//vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//vertex texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void cleanup() {

    // GPU cleanup
    glDeleteBuffers(1, &verticesVBO);
    glDeleteBuffers(1, &verticesEBO);
    glDeleteVertexArrays(1, &objectVAO);
    glDeleteTextures(1, &texture);

    glfwDestroyWindow(glWindow);
    //close GL context and any other GLFW resources
    glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
	    glfwTerminate();
	    return 1;
	}

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	
	//adaugate
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//adaugate - pentru transparenta

	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	myCustomShader.loadShader("D:/FACULTATE/PG/lab/proiect/lab6/shaders/shaderStart.vert", "D:/FACULTATE/PG/lab/proiect/lab6/shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();

	//initialize the projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(55.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//loadTriangleData();
	//texture = ReadTextureFromFile("D:/FACULTATE/PG/lab/L6/lab6/textures/hazard2.png");
	
	//myModel.LoadModel("D:/FACULTATE/PG/lab/L6/lab6/objects/stall.obj");
	//texture = ReadTextureFromFile("D:/FACULTATE/PG/lab/L6/lab6/textures/stall_texture.png");

	//myModel.LoadModel("D:/FACULTATE/PG/lab/L6/lab6/objects/movila.obj", "D:/FACULTATE/PG/lab/L6/lab6/textures/");
	myModel.LoadModel("D:/FACULTATE/PG/lab/proiect/lab6/objects/proiect_solid.obj", "D:/FACULTATE/PG/lab/proiect/lab6/textures/");

	while (!glfwWindowShouldClose(glWindow)) {
        processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
