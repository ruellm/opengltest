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

GLenum error;
int number = 40;


void LoadTexture(GLuint* textureID, const char* path)
{
	glGenTextures(1, textureID);
	glBindTexture(GL_TEXTURE_2D, *textureID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data)
	{
		GLuint format = GL_RGB;
		if (nrChannels == 4)
			format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	error = glGetError();
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	float vertices[] = {
		0.5f,	0.5f,		// center

		0.5f,	0,
		0.25f,	0,
		0,		0,

		0,		0.25f,
		0,		0.5f,
		0,		0.75f,
		0,		1,

		0.25f,	1,
		0.5f,	1,
		0.75f,	1,
		1,		1,

		1,		0.75f,
		1,		0.5f,
		1,		0.25f,
		1,		0,

		0.75f,	0,
		0.5f,	0
	};

	float num_of_vertices = sizeof(vertices) / sizeof(float);
	float total_vertices = num_of_vertices / 2.0f;

	GLuint VAO = 0;
	GLuint VBO = 0;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num_of_vertices,
		vertices, GL_STATIC_DRAW);

	//You don't need to specify a stride(5th param) if you are using a single attribute,
	//since there is no gap between the data associated with the vertices. 
	//If you have multiple attributes, you need to use offset and stride to tell opengl how to access them.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0);

	GLShaderFx shader;
	shader.Init();
	shader.LoadVertexShaderFromFile("./shaders/basic_vshader_nocolor");
	shader.LoadFragmentShaderFromFile("./shaders/solid_color_fshader");
	shader.Build();

	glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);

	// render loop
	// -----------
	double lastTime = glfwGetTime(), timer = lastTime;
	double elapsed = 0, nowTime = 0;


	float pct = 1.0f;

	while (!glfwWindowShouldClose(window))
	{
		// - Measure time
		nowTime = glfwGetTime();
		elapsed = (nowTime - lastTime) / 1000.0f;
		lastTime = nowTime;

		// input
		// -----
		processInput(window);


		// Update
		pct -= 100 * elapsed;
		if (pct <= 0.0f)
			pct = 1.0f;

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Use();

		// Prepare transformations
		glm::mat4 model = glm::mat4(1.0f);	//initialize to identity

		// First translate (transformations are: scale happens first,
		// then rotation and then finall translation happens; reversed order)
		model = glm::translate(model, glm::vec3(100, 100, 0.0f));

		//Resize to current scale
		model = glm::scale(model, glm::vec3(100, 100, 1.0f));

		int projmtx = shader.GetUniformLocation("projection");
		shader.SetUniformMatrix(projmtx, projection);

		int modelmtx = shader.GetUniformLocation("model");
		shader.SetUniformMatrix(modelmtx, model);

		int colorID = shader.GetUniformLocation("color");
		shader.SetUniformVector3(colorID, glm::vec3(0.1, 0.1, 0.1));

		int alphaID = shader.GetUniformLocation("alpha");
		shader.SetUniformFloat(alphaID,0.5f);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, total_vertices * pct);

		glDisable(GL_BLEND);
		
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