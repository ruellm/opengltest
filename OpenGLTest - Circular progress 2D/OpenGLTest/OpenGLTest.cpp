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

void BuildCircle(std::vector<glm::vec2>& list, int radius)
{
	// 1. build circle mask using triangle fan points
	// reference https://www.youtube.com/watch?v=b1le0hAYJSI
	std::vector<glm::vec2> circle_vertices;

	float twopi = 2.0f * glm::pi<float>();

	// insert the center
	glm::vec2 center = glm::vec2(
		SCR_WIDTH / 2,
		SCR_HEIGHT / 2);

	list.push_back(center);

	for (int i = 0; i <= number; i++)
	{
		float x = radius * cos(i*twopi / number);
		float y = radius * sin(i*twopi / number);
		list.push_back(center + glm::vec2(x, y));
	}

}

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


	GLShaderFx cshader;
	cshader.Init();
	cshader.LoadVertexShaderFromFile("./shaders/mask_vshader.txt");
	cshader.LoadFragmentShaderFromFile("./shaders/mask_fshader.txt");
	cshader.Build();

	cshader.Use();							// use shader/program, glUniform1i generates error if no program use

	int modelmtx = cshader.GetUniformLocation("model");
	int projmtx = cshader.GetUniformLocation("projection");

	glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);
	glm::mat4 modelmat = glm::mat4(1.0f);	//initialize to identity

	cshader.SetUniformMatrix(modelmtx, modelmat);
	cshader.SetUniformMatrix(projmtx, projection);

	// render loop
	// -----------
	double lastTime = glfwGetTime(), timer = lastTime;
	double elapsed = 0, nowTime = 0;


	
	GLuint cVAO = 0;
	GLuint cVBO = 0;
	std::vector<glm::vec2> circle_vertices;
	BuildCircle(circle_vertices, 180);

	glGenVertexArrays(1, &cVAO);
	glGenBuffers(1, &cVBO);

	glBindVertexArray(cVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * circle_vertices.size(), 
		&circle_vertices[0], GL_STATIC_DRAW);

	//You don't need to specify a stride if you are using a single attribute,
	//since there is no gap between the data associated with the vertices. 
	//If you have multiple attributes, you need to use offset and stride to tell opengl how to access them.
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	GLuint cVAO_small = 0;
	GLuint cVBO_small = 0;
	std::vector<glm::vec2> circle_vertices_small;
	BuildCircle(circle_vertices_small, 150);


	std::vector<glm::vec2> glow_pos;
	BuildCircle(glow_pos, 155);

	glGenVertexArrays(1, &cVAO_small);
	glGenBuffers(1, &cVBO_small);

	glBindVertexArray(cVAO_small);
	glBindBuffer(GL_ARRAY_BUFFER, cVBO_small);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * circle_vertices_small.size(),
		&circle_vertices_small[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

#if 1
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	// define in screen space -- because we are using ortho projection
	float vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};


	unsigned int VBO = 0;
	unsigned int VAO = 0;

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#endif

	GLShaderFx shader;
	shader.Init();
	shader.LoadVertexShaderFromFile("./shaders/basic_vshader.txt");
	shader.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	shader.Build();


	unsigned int texture1 = 0;
	unsigned int texture2 = 0;
	LoadTexture(&texture1, "./textures/circular_back.png");
	LoadTexture(&texture2, "./textures/timer glow.png");

	//texture handling, activate texture unit 1
	//Using glUniform1i we can actually assign a location value to the texture 
	//sampler so we can set multiple textures at once in a fragment shader. 
	//This location of a texture is more commonly known as a texture unit.
	//The main purpose of texture units is to allow us to use more than 1 texture in our shaders.

	shader.Use();							// use shader/program, glUniform1i generates error if no program use

	int textureID = shader.GetUniformLocation("texture");
	glUniform1i(textureID, 0);				// set the textureID to texture unit 0,
											// Note: GL_INVALID_OPERATION is generated if there is no current program object. 

	glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
	glBindTexture(GL_TEXTURE_2D, texture1);


	modelmtx = shader.GetUniformLocation("model");
	projmtx = shader.GetUniformLocation("projection");

	// translate the sprite to x,y
	float X = (SCR_WIDTH/2)-(326/2), Y = SCR_HEIGHT/2-325/2;
	modelmat = glm::translate(modelmat, glm::vec3(X, Y, 0.0f));

	//resize the primitize to sprite size, sample is 512x512
	modelmat = glm::scale(modelmat, glm::vec3(326, 326, 1.0f));

	shader.SetUniformMatrix(modelmtx, modelmat);
	shader.SetUniformMatrix(projmtx, projection);


	float pct = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		// - Measure time
		nowTime = glfwGetTime();
		elapsed = (nowTime - lastTime) / 1000.0f;
		lastTime = nowTime;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

#if 1
		// Stencil reference test
		// https://stackoverflow.com/questions/32818060/opengl-stencil-test-in-a-rectangle
		// https://en.wikibooks.org/wiki/OpenGL_Programming/Stencil_buffer#Sample
		glEnable(GL_STENCIL_TEST);
		// Fill stencil buffer with 0's
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);

		// Write 1's into stencil buffer where the hole will be
		// disable draw to framebuffer and depth buffer
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		// now that all values in stencil buffer is 0, always pass the result
		// and replace value when it pass and depth buffer is disabled (3rd option of glStencilOp)
		// (ref & mask) OP (stencil & mask)
		// (1 & 0xff) always (0 & 0xfff) = always pass, so use 3rd option of glStencilOp
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
#endif
#if 1
		cshader.Use();



		pct += (100) * elapsed;
		pct = (pct <= 0.0f) ? 1.0 : pct;

		// compute the current mask
		float pct_per_fan = 1 / (float)(number+1);
		float fan_count = pct * (number + 1);

		// draw the huge circle mask
		glBindVertexArray(cVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, ++fan_count);

		// draw the smaller circle mask
		glBindVertexArray(cVAO_small);
		glDrawArrays(GL_TRIANGLE_FAN, 0, circle_vertices_small.size());
#endif
#if 1
		// Enable color frame and depth buffer rendering
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		// Pass if 
		// stencil formula is (ref & mask) OP (stencil & mask)
		// therefore (1 & 0xFF) != stencil & 0xff
		// so if there is a value in stencil (inside the mask)
		// the value is (1 & 0xFF) != (1 & 0xff), 1 != 1 which is fail, stencel failed so keep values (1st parameter in glStencilOp)
		// if value in stencil is not in mask (1 & 0xFF) != (0 & 0xff), 1 != 0, pass, keep the value and not replace since we are done with stencil test 
		// (3rd parameter in glStencilOp)
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		shader.Use();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		modelmat = glm::mat4(1.0f);
		modelmat = glm::translate(modelmat, glm::vec3(X, Y, 0.0f));
		modelmat = glm::scale(modelmat, glm::vec3(326, 326, 1.0f));
		shader.SetUniformMatrix(modelmtx, modelmat);
		glBindTexture(GL_TEXTURE_2D, texture1);

		// 3. now draw the object 
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_STENCIL_TEST);
		// translate the sprite to x,y	float X = (SCR_WIDTH/2)-(326/2), Y = SCR_HEIGHT/2-325/2;
		glm::vec2 coord = glow_pos[fan_count - 1];
		modelmat = glm::mat4(1.0f);
		int width = 23;
		int height = 23;
		int newx = coord.x - width / 2;
		int newy = coord.y - height / 2;
		modelmat = glm::translate(modelmat, glm::vec3(newx, newy, 0.0f));
		modelmat = glm::scale(modelmat, glm::vec3(width, height, 1.0f));
		shader.SetUniformMatrix(modelmtx, modelmat);

		glBindTexture(GL_TEXTURE_2D, texture2);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

#endif
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glDeleteVertexArrays(1, &cVAO);
	glDeleteBuffers(1, &cVBO);

	glDeleteVertexArrays(1, &cVAO_small);
	glDeleteBuffers(1, &cVBO_small);
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