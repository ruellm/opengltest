// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")

#include "common.h"
#include <iostream>
#include "GLShaderFx.h"
#include "stb_image.h"
#include "Camera.h"
#include "obj_model.h"

// settings
const unsigned int SCR_WIDTH	= 800;
const unsigned int SCR_HEIGHT	= 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

GLenum error;

//
// for 2D demo
//
unsigned int VBO_2D = 0;
unsigned int VAO_2D = 0;
GLShaderFx shader2D;
unsigned int textureWall;

// 3D image demo
unsigned int VBO_text = 0;
unsigned int VAO_text = 0;
GLShaderFx shaderText;
unsigned int textureText;

// Projection matrix 4:3 ratio
glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f),
	4.0f / 3.0f, 0.1f, 100.0f);

// Camera matrix
glm::mat4 viewMatrix;

glm::vec3 _campos;
Camera camera;

void SetUp3DObject()
{
	float vertices[] = {
		// 3D Coordinates	texture coordinates, 
		// in windows, image is upside down, therefore invert texture coordinates
		2.0f,  2.0f, 0.0f,		1.0f, 0.0f,		// top right
		2.0f, -2.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-2.0f,  2.0f, 0.0f,		0.0f, 0.0f,		// top left 
												// second triangle
		2.0f, -2.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-2.0f, -2.0f, 0.0f,		0.0f, 1.0f,		// bottom left
		-2.0f,  2.0f, 0.0f,		0.0f, 0.0f		// top left
	};

	glGenBuffers(1, &VBO_text);
	glGenVertexArrays(1, &VAO_text);

	glBindVertexArray(VAO_text);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_text);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shaderText.Init();
	shaderText.LoadVertexShaderFromFile("./shaders/vshader_world.txt");
	shaderText.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	shaderText.Build();
	shaderText.Use();

	glGenTextures(1, &textureText);
	glBindTexture(GL_TEXTURE_2D, textureText);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load("./textures/sample2.png", &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	error = glGetError();

	stbi_image_free(data);


}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR	, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR	, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE			, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	
	SetUp3DObject();

	camera.LookAt(glm::vec3(0, 0, 10), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));

	OBJModel hound;
	hound.LoadFromFile("Hellhound_OBJ.obj", "models");
	GLShaderFx meshShader;
	meshShader.Init();
	meshShader.LoadVertexShaderFromFile("./shaders/model_vshader.txt");
	meshShader.LoadFragmentShaderFromFile("./shaders/model_fshader.txt");
	meshShader.Build();

	// render loop
	// -----------
	int textureID = 0;
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		camera.Update();
		viewMatrix = camera.GetViewMatrix();
		hound.Update(0.1f);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		meshShader.Use();
		int worldID = meshShader.GetUniformLocation("gWorld");
		int VPID = meshShader.GetUniformLocation("gVP");
		glm::mat4 world = glm::mat4(1.0f);
		world = glm::translate(world, glm::vec3(-10, 0, 0));

		shaderText.SetUniformMatrix(worldID, world);
		shaderText.SetUniformMatrix(VPID, projectionMatrix * viewMatrix);
		hound.Draw();

#if 1
		// Setup projection/camera
		glm::mat4 model = glm::mat4(1.0f);		//identity
		shaderText.Use();

		int modelID = shaderText.GetUniformLocation("model");
		int projectionID = shaderText.GetUniformLocation("projection");
		int viewID = shaderText.GetUniformLocation("view");

		int billboardPosID = shaderText.GetUniformLocation("BillboardPos");
		shaderText.SetUniformVector3(billboardPosID, glm::vec3(0, 0, 0));

		int billboardSize = shaderText.GetUniformLocation("BillboardSize");
		shaderText.SetUniformVector2(billboardSize, glm::vec2(2, 2));

		int cameraRightWorldID = shaderText.GetUniformLocation("CameraRight_worldspace");
		shaderText.SetUniformVector3(cameraRightWorldID, glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));

		int cameraUpWorldID = shaderText.GetUniformLocation("CameraUp_worldspace");
		shaderText.SetUniformVector3(cameraUpWorldID, glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));

		shaderText.SetUniformMatrix(modelID, model);
		shaderText.SetUniformMatrix(projectionID, projectionMatrix);
		shaderText.SetUniformMatrix(viewID, viewMatrix);


		// RENDER transparent text
		shaderText.Use();
		error = glGetError();

		textureID = shaderText.GetUniformLocation("texture");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
		glBindTexture(GL_TEXTURE_2D, textureText);

		// Enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(VAO_text);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBindVertexArray(0);
#endif


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.Walk(-0.10f);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.Walk(0.10f);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.Strafe(-0.05f);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.Strafe(0.05f);
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		camera.Yaw(0.001f);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		camera.Yaw(-0.001f);
	}
}