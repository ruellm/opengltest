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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

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

#if 1
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// first triangle
		// clip space points	texture coordinates
		0.5f,  0.5f, 0.0f,		1.0f, 1.0f,		// top right
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		// bottom right
		-0.5f,  0.5f, 0.0f,		0.0f, 1.0f,		// top left 
												// second triangle
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		// bottom right
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		// bottom left
		-0.5f,  0.5f, 0.0f,		0.0f, 1.0f		// top left
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
#endif
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

	//texture handling, activate texture unit 1
	//Using glUniform1i we can actually assign a location value to the texture 
	//sampler so we can set multiple textures at once in a fragment shader. 
	//This location of a texture is more commonly known as a texture unit.
	//The main purpose of texture units is to allow us to use more than 1 texture in our shaders.

	int textureID = shader.GetUniformLocation("texture");
	glUniform1i(textureID, 0);				// set the textureID to texture unit 0
	glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
	
	glfwSetMouseButtonCallback(window, mouse_button_callback);


	//-------------------------------------------------------------------------
	// full screen rectangle
	float screen_vet[] = {
		// first triangle
		// clip space points	texture coordinates
		1.0f,  1.0f, 0.0f,		1.0f, 1.0f,		// top right
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f,		// bottom right
		-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,		// top left 
												// second triangle
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f,		// bottom right
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,		// bottom left
		-1.0f,  1.0f, 0.0f,		0.0f, 1.0f		// top left
	};

	unsigned int VBO_scr = 0;
	unsigned int VAO_scr = 0;

	glGenBuffers(1, &VBO_scr);
	glGenVertexArrays(1, &VAO_scr);

	glBindVertexArray(VAO_scr);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_scr);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vet), screen_vet, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	int test_scree_id = shader.GetUniformLocation("isScreen");
	int alpha_id = shader.GetUniformLocation("alpha");
	float alpha = 1.0f;

	// render loop
	// -----------
	double lastTime = glfwGetTime(), timer = lastTime;
	double elapsed = 0, nowTime = 0;
	int dir = 0;

	while (!glfwWindowShouldClose(window))
	{
		// - Measure time
		nowTime = glfwGetTime();
		elapsed = (nowTime - lastTime) ;
		lastTime = nowTime;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glBindTexture(GL_TEXTURE_2D, texture1);

		shader.Use();
		shader.SetUniformInt(test_scree_id, 0);

		// 3. now draw the object 
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		if (dir == 0)
		{
			if (alpha > 0) {
				alpha -= (1.0f * elapsed);
			}
			else {
				alpha = 0;
			}
		}

		printf("\n alpha = %.2f", alpha);
		//Render the fade in rectangle
		shader.SetUniformInt(test_scree_id, 1);
		shader.SetUniformFloat(alpha_id, alpha);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		glBindVertexArray(VAO_scr);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE);
		
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		printf("\n Left mouse button pressed %f %f", xpos, ypos);

	}
}