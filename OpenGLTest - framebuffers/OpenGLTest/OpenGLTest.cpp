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
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// clip space point		//text coord
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, // left  
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f, // right 
		0.0f,  0.5f, 0.0f,		0.5f, 1.0f // top   
	};

	unsigned int VBO = 0;
	unsigned int VAO = 0;

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	GLShaderFx shader;
	shader.Init();
	shader.LoadVertexShaderFromFile("./shaders/basic_vshader.txt");
	shader.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	shader.Build();

	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
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


	//-----------------------------------------------
	// Render to Texture demo - can be use as temporary buffer
	// source referencese:
	//  https://open.gl/framebuffers
	//  http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
	//  https://software.intel.com/en-us/articles/dynamic-resolution-rendering-on-opengl-es-2

	int working_width = 1024;
	int working_height = 1080;

	// 1. create frame buffer
	GLuint frameBuffer = 0;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);	

	// 2. create a blank texture which will contain the RGB output of our shader.
	// data is set to NULL
	GLuint texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB, working_width, working_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
		);

	error = glGetError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 3. attached our texture to the frame buffer, note that our custom frame buffer is already active
	// by glBindFramebuffer
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0
		);

	error = glGetError();

	// 4. we create the depth and stencil buffer also, (this is optional)
#if 0
	GLuint rboDepthStencil;
	glGenRenderbuffers(1, &rboDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, working_width, working_height);
#endif

	error = glGetError();

	// Set the list of draw buffers. this is not needed?
	//GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	error = glGetError();
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "\nFrame buffer error not complete";
	}
	// Restore frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	error = glGetError();

	// create our Rectangle
	float screen_rect[] = {
		// clip space points	texture coordinates
		1.0f,  1.0f, 0.0f,		1.0f, 1.0f,		// top right
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f,		// bottom right
		-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,		// top left 
												// second triangle
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f,		// bottom right
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,		// bottom left
		-1.0f,  1.0f, 0.0f,		0.0f, 1.0f		// top left
	};

	unsigned int scrrect_VBO = 0;
	unsigned int  scrrect_VAO = 0;

	glGenBuffers(1, &scrrect_VBO);
	glGenVertexArrays(1, &scrrect_VAO);

	glBindVertexArray(scrrect_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, scrrect_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_rect), screen_rect, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	error = glGetError();

	//texture handling, activate texture unit 1
	//Using glUniform1i we can actually assign a location value to the texture 
	//sampler so we can set multiple textures at once in a fragment shader. 
	//This location of a texture is more commonly known as a texture unit.
	//The main purpose of texture units is to allow us to use more than 1 texture in our shaders.

	shader.Use();		// a program needs to be loaded for glUniform1i to work

	int textureID = shader.GetUniformLocation("texture");
	glUniform1i(textureID, 0);				// set the textureID to texture unit 0
	glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
	glBindTexture(GL_TEXTURE_2D, texture1);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------

#if 1
		// Render to our frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, working_width, working_height);		// use the entire texture,
																// this means that use the dimension set as our total 
																// display area

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Use();
		glBindTexture(GL_TEXTURE_2D, texture1);

		// 3. now draw the object 
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);


#endif
		//-----------------------------------------------------------------------------
#if 1
		// Restore frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// activate the texture unit 0 first before binding texture
		shader.Use();

		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);	// bind the render target texture


		glBindVertexArray(scrrect_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		error = glGetError();
#endif

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteTextures(1, &texColorBuffer);
	glDeleteTextures(1, &texture1);
#if 0
	glDeleteRenderbuffers(1, &rboDepthStencil);
#endif

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