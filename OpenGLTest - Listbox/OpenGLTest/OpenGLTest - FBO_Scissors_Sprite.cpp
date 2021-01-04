// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")

#include "common.h"
#include <iostream>
#include "GLShaderFx.h"
#include "stb_image.h"

#include "FBO.h"

// settings
const unsigned int SCR_WIDTH	= 800;
const unsigned int SCR_HEIGHT	= 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

GLenum error;

GLuint _quadVAO = 0;
GLuint _VBO = 0;

// render target to main screen
// coordiantes are inverted
GLuint _quadVAO_RT = 0;
GLuint _VBO_RT = 0;

GLShaderFx spriteShader;
GLuint hayleyTex = 0;

glm::mat4 projection = glm::mat4(1.0f);

void Sprite2D_InitData()
{
	// texture coordinate is inverted because we are using ortho?
	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f,	1.0f, 0.0f,
		0.0f, 0.0f,	0.0f, 0.0f,

		0.0f, 1.0f,	0.0f, 1.0f,
		1.0f, 1.0f,	1.0f, 1.0f,
		1.0f, 0.0f,	1.0f, 0.0f
	};

	glGenVertexArrays(1, &_quadVAO);
	glGenBuffers(1, &_VBO);

	glBindVertexArray(_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Sprite2D_InitData_RT()
{
	// texture coordinate is inverted because we are using ortho?
	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f,  0.0f, 0.0f,
		1.0f, 0.0f,	 1.0f, 1.0f,
		0.0f, 0.0f,	 0.0f, 1.0f,

		0.0f, 1.0f,	 0.0f, 0.0f,
		1.0f, 1.0f,	 1.0f, 0.0f,
		1.0f, 0.0f,	 1.0f, 1.0f
	};

	glGenVertexArrays(1, &_quadVAO_RT);
	glGenBuffers(1, &_VBO_RT);

	glBindVertexArray(_quadVAO_RT);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO_RT);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Sprite2D_Draw(int x, int y, int width, int height, 
	GLuint texture, GLuint VAO, GLuint VBO)
{
	// Prepare transformations
	glm::mat4 model = glm::mat4(1.0f);	//initialize to identity

										// First translate (transformations are: scale happens first,
										// then rotation and then finall translation happens; reversed order)

	model = glm::translate(model, glm::vec3(x, y, 0.0f));

	//Resize to current scale
	model = glm::scale(model, glm::vec3(width, height, 1.0f));

	spriteShader.Use();

	int projmtx = spriteShader.GetUniformLocation("projection");
	spriteShader.SetUniformMatrix(projmtx, projection);

	int modelmtx = spriteShader.GetUniformLocation("model");
	spriteShader.SetUniformMatrix(modelmtx, model);

	int textureID = spriteShader.GetUniformLocation("texture");
	glUniform1i(textureID, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);


	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint LoadTexture(const char* fnzme)
{
	GLuint texture = 0;
	
	//...
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(fnzme, &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	stbi_image_free(data);

	return texture;
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

	projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);

	FBO frameBuffer;
	frameBuffer.Initialize(800, 600);	

	spriteShader.Init();
	spriteShader.LoadVertexShaderFromFile("./shaders/basic_vshader.txt");
	spriteShader.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	spriteShader.Build();

	hayleyTex = LoadTexture("./textures/smiley.jpg");
	Sprite2D_InitData();
	Sprite2D_InitData_RT();

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
		frameBuffer.Bind();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		int yoffset = -100;
		//scissors
		glEnable(GL_SCISSOR_TEST);

		// in actual object x, y coordinates of a 2D object is specified, therefore we set the lower left to y+height
		// Y is lower left of the box itself
		int y = 0, height = 100;

		int actualy = (SCR_HEIGHT - y) - height;		// on this equation Y is top left,
		glScissor(0, actualy, 480, height);				//y is the lower left, the scissor of the viewport

		//... display backbuffer contents here
		//draw sprite at backbuffer 0,0
		Sprite2D_Draw(0, yoffset, 480, 480, hayleyTex, _quadVAO, _VBO);

		//glDisable(GL_SCISSOR_TEST);

		//-----------------------------------------------------------------------------
		// Restore frame buffer		
		frameBuffer.Unbind(SCR_WIDTH, SCR_HEIGHT);
#endif

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//display to main screen
		Sprite2D_Draw(0, 0, 800, 600, frameBuffer.GetColorTexture(), _quadVAO_RT, _VBO_RT);

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