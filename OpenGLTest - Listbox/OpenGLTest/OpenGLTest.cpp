// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")

#include "common.h"
#include <iostream>
#include "GLShaderFx.h"
#include "stb_image.h"
#include "Listbox.h"
#include "FBO.h"
#include "uimanager.h"

// settings
//const unsigned int SCR_WIDTH	= 800;
//const unsigned int SCR_HEIGHT	= 600;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

GLenum error;

GLShaderFx spriteShader;
glm::mat4 projection = glm::mat4(1.0f);

GLuint _quadVAO = 0;
GLuint _VBO = 0;

// render target to main screen
// coordiantes are inverted
GLuint _quadVAO_RT = 0;
GLuint _VBO_RT = 0;

GLuint hayleyTex = 0;
UIManager uimanager;

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
	// texture coordinate is inverted because Render target 
	// is using screen space coordinate
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

void Sprite2D_Draw(int x, int y, int width, int height, GLuint texture)
{
	Sprite2D_Draw(x, y, width, height, texture, _quadVAO, _VBO);
}

void Sprite2D_DrawRT(int x, int y, int width, int height, GLuint texture)
{
	Sprite2D_Draw(x, y, width, height, texture, _quadVAO_RT, _VBO_RT);
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Sprite2D_InitData();
	Sprite2D_InitData_RT();

	projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);

	spriteShader.Init();
	spriteShader.LoadVertexShaderFromFile("./shaders/basic_vshader.txt");
	spriteShader.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	spriteShader.Build();

	Listbox listbox;
	listbox.Initialize(SCR_WIDTH, 47*10);
	listbox._x = 10;
	listbox._y = 10;

	uimanager.Add(&listbox);

	// render loop
	// -----------
	double lastTime = glfwGetTime(), timer = lastTime;
	double elapsed = 0, nowTime = 0;
	while (!glfwWindowShouldClose(window))
	{
		// - Measure time
		nowTime = glfwGetTime();
		elapsed = (nowTime - lastTime);
		lastTime = nowTime;

		uimanager.OnUpdate(elapsed);

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		uimanager.OnDraw();

		//display to main screen

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

int button_down = false;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			uimanager.OnMouseDown((int)xpos,(int)ypos);

			//printf("\n Mouse down coordinates %f %f", xpos, ypos);
		}
		else if (action == GLFW_RELEASE) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			uimanager.OnMouseUp((int)xpos, (int)ypos);
		}
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	//if (button_down) {
		// mouse move
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		uimanager.OnMouseMove((int)xpos, (int)ypos);

		//vertices.push_back(xpos);
		//vertices.push_back(ypos);
		//printf("\n Drag coordinates %f %f", xpos, ypos);
	}
}