// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")

#include "common.h"
#include <iostream>
#include "GLShaderFx.h"
#include "stb_image.h"

#include "Stroking2D.h"

// settings
const unsigned int SCR_WIDTH	= 800;
const unsigned int SCR_HEIGHT	= 600;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

GLenum error;

struct PointInfo
{
	int start_index;
	int count;
};

struct LineInfo
{
	std::vector<Point2D> points;
};
int current = 0;

std::vector<struct LineInfo> lines;

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

void StrokeLine(std::vector<Point2D>& points)
{
	if (points.size() < 2) return;

	std::vector<Point2D> vertices;
	LineStrokeAttribute attr;

	attr.cap = STROKE_LINECAP_BUTT;
	attr.width = 10;
	attr.miterLimit = 10;
	attr.join = STROKE_JOIN_ROUND;

	GetStrokeGeometry(points, attr, vertices);

	unsigned int stroke_VAO = 0;
	unsigned int stroke_VBO = 0;

	glGenVertexArrays(1, &stroke_VAO);
	glGenBuffers(1, &stroke_VBO);

	glBindVertexArray(stroke_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, stroke_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Point2D) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	//You don't need to specify a stride if you are using a single attribute,
	//since there is no gap between the data associated with the vertices. 
	//If you have multiple attributes, you need to use offset and stride to tell opengl how to access them.
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(stroke_VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDeleteBuffers(1, &stroke_VBO);
	glDeleteVertexArrays(1, &stroke_VAO);
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


	GLShaderFx shader;
	shader.Init();
	shader.LoadVertexShaderFromFile("./shaders/basic_vshader.txt");
	shader.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	shader.Build();


	error = glGetError();

	//texture handling, activate texture unit 1
	//Using glUniform1i we can actually assign a location value to the texture 
	//sampler so we can set multiple textures at once in a fragment shader. 
	//This location of a texture is more commonly known as a texture unit.
	//The main purpose of texture units is to allow us to use more than 1 texture in our shaders.

	shader.Use();							// use shader/program, glUniform1i generates error if no program use

	
	int modelmtx = shader.GetUniformLocation("model");
	int projmtx = shader.GetUniformLocation("projection");
	
	glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f, -1.0f, 1.0f);
	glm::mat4 modelmat = glm::mat4(1.0f);	//initialize to identity

	
	shader.SetUniformMatrix(modelmtx, modelmat);
	shader.SetUniformMatrix(projmtx, projection);

	// render loop
	// -----------
	double lastTime = glfwGetTime(), timer = lastTime;
	double elapsed = 0, nowTime = 0;

	/*GLuint penTexture;
	LoadTexture(&penTexture, "./textures/pentext.png");
	int textureID = shader.GetUniformLocation("gSampler");

	glUniform1i(textureID, 0);				// set the textureID to texture unit 0
	glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
	glBindTexture(GL_TEXTURE_2D, penTexture);
	*/

	int resolution = shader.GetUniformLocation("resolution");
	glm::vec2 viewport = glm::vec2(SCR_WIDTH, SCR_HEIGHT);
	shader.SetUniformVector2(resolution, viewport);

	// for wireframe mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#if 0 // wireframe unit testing

	std::vector<Point2D> points;	
	// Sample 1
   //points.push_back(Point2D(30, 268));
	//points.push_back(Point2D(29, 267));
	//points.push_back(Point2D(27, 255));
	//points.push_back(Point2D(27, 242));

	// Sample 2
//	points.push_back(Point2D(100, 100));
//	points.push_back(Point2D(200, 100));
//	points.push_back(Point2D(300, 100));
	//points.push_back(Point2D(400, 100));

	// sample 3
	//points.push_back(Point2D(500, 100));
	//points.push_back(Point2D(100, 300));
	//points.push_back(Point2D(400, 500));

#endif
	std::vector<Point2D> points;

	Point2D p1;
	Point2D p2;

	p1._x = 0;
	p1._y = 100;

		p1._x = 49;
		p2._y = 100-78;
	points.push_back(p1);
	points.push_back(p2);

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

		StrokeLine(points);
#if 0
		for (int i = 0; i < lines.size(); i++) {
			StrokeLine(lines[i].points);
		}
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
}

int button_down = false;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT )
	{
		if (action == GLFW_PRESS) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			if (lines.size() == 0) {
				struct LineInfo line;
				lines.push_back(line);
			}

			struct LineInfo* line = &lines.at(current);
			line->points.push_back(Point2D(xpos, ypos));

			button_down = true;

			printf("\n Mouse down coordinates %f %f", xpos, ypos);			
		}

		else if (action == GLFW_RELEASE) {
			struct LineInfo line;
			lines.push_back(line);
			current++;

			button_down = false;
		}
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (button_down) {
		// mouse move
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		
		struct LineInfo* line = &lines.at(current);
		line->points.push_back(Point2D(xpos, ypos));
		
		printf("\n Drag coordinates %f %f", xpos, ypos);
	}
}