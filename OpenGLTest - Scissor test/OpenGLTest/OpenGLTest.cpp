// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")

#include "common.h"
#include <iostream>
#include "GLShaderFx.h"
#include "stb_image.h"

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

void SetUp2DObject()
{
	// SETUP 2D drawing
	// set up vertex data (and buffer(s)) and configure vertex attributes
	float vertices[] = {
		// clip space points(NDC)	texture coordinates
		// in windows, image is upside down, therefore invert texture coordinates
		1.0f,  1.0f, 0.0f,		1.0f, 0.0f,		// top right
		1.0f, -1.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-1.0f,  1.0f, 0.0f,		0.0f, 0.0f,		// top left 
												// second triangle
		1.0f, -1.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-1.0f, -1.0f, 0.0f,		0.0f, 1.0f,		// bottom left
		-1.0f,  1.0f, 0.0f,		0.0f, 0.0f		// top left
	};

	glGenBuffers(1, &VBO_2D);
	glGenVertexArrays(1, &VAO_2D);

	glBindVertexArray(VAO_2D);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2D);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader2D.Init();
	shader2D.LoadVertexShaderFromFile("./shaders/basic_vshader.txt");
	shader2D.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	shader2D.Build();


	glGenTextures(1, &textureWall);
	glBindTexture(GL_TEXTURE_2D, textureWall);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load("./textures/wall.jpg", &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	stbi_image_free(data);
}

void SetUp3DObject()
{
	

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR	, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE			, GLFW_OPENGL_ANY_PROFILE);

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

	
	SetUp2DObject();
	SetUp3DObject();

	// Important! Since cube does not have a back side, disable culling
	glCullFace(GL_FRONT_AND_BACK);
	// render loop
	// -----------
	int textureID = 0;
	int yrunning = 0;

	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
#if 1
		// RENDER 2D wall
		shader2D.Use();
		textureID = shader2D.GetUniformLocation("texture");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
		glBindTexture(GL_TEXTURE_2D, textureWall);

		glBindVertexArray(VAO_2D);
		glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
		glEnable(GL_SCISSOR_TEST);
		// in actual object x, y coordinates of a 2D object is specified, therefore we set the lower left to y+height
		// Y is lower left of the box itself
		int y = 0, height = 175;

		int actualy = (SCR_HEIGHT - y) - height;	// on this equation Y is top left,
		glScissor(0, actualy, 100, height); //y is the lower left, the scissor of the viewport


		glBindTexture(GL_TEXTURE_2D, textureText);
		glBindVertexArray(VAO_2D);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_SCISSOR_TEST);


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