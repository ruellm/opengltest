// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")

#include "common.h"
#include <iostream>
#include "GLShaderFx.h"
#include "stb_image.h"
#include "animated_sprite.h"

// settings
const unsigned int SCR_WIDTH	= 800;
const unsigned int SCR_HEIGHT	= 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// Projection matrix 4:3 ratio
glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f),
	4.0f / 3.0f, 1.0f, 100.0f);

// Camera matrix
glm::mat4 viewMatrix = glm::lookAt(
	glm::vec3(0, 0, 20), // Camera is at 0,0,5 in World Space, our triangle is at 0 z axis
	glm::vec3(0, 0, 0), // and looks at the origin
	glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

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


	AnimatedSprite sprite;
	sprite.Load("textures/lightning_0.png");
	sprite.Set(8, 10.0f, true, 1024 / 8);

	GLShaderFx shader;
	shader.Init();
	shader.LoadVertexShaderFromFile("./shaders/animated_sprite.vs");
	shader.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	shader.Build();
	shader.Use();

	int modelID = shader.GetUniformLocation("model");
	int VP = shader.GetUniformLocation("VP");

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(2, 8, 1));

	shader.SetUniformMatrix(modelID, model);

	glm::mat4 VPmat= projectionMatrix * viewMatrix;
	shader.SetUniformMatrix(VP, VPmat);
	int frameWidthID = shader.GetUniformLocation("frameWidth");

	float frameWidth = sprite.GetFrameWidthNormalized();
	shader.SetUniformFloat(frameWidthID, frameWidth);

	// render loop
	// -----------
	int textureID = 0;
	float lasttime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		float current_time = glfwGetTime();
		float elapsed = (current_time - lasttime);
		lasttime = current_time;

		sprite.Update(elapsed);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Use();

		int currentFrameID = shader.GetUniformLocation("currentFrame");
		int frameWidthID = shader.GetUniformLocation("frameWidth");

		float frameWidth = sprite.GetFrameWidthNormalized();
		shader.SetUniformFloat(frameWidthID, frameWidth);
		shader.SetUniformInt(currentFrameID, sprite.GetCurrentFrame());

		textureID = shader.GetUniformLocation("texture");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0

		sprite.Draw();

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
}