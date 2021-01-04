// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")

#include "common.h"
#include <iostream>
#include <string>
#include "GLShaderFx.h"
#include "stb_image.h"

// N$ 
#include "lib/GeometricRecognizer.h"
#include "lib/PathWriter.h"
#include "lib/GeometricRecognizerTypes.h"

using namespace DollarRecognizer;

// P$ 
#include "pdollar/Geometry.h"
#include "pdollar/Gesture.h"
#include "pdollar/PointCloudRecognizer.h"

using namespace PDollarGestureRecognizer;
Gesture GetTGestures();
Gesture GetSGestures();
Gesture GetS11_1();
Gesture GetS12_1();

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

std::vector<struct PointInfo> Points;
std::vector<float>	vertices;
int start_index = 0;
GLShaderFx shader_FB;
struct LineInfo
{
	std::vector<float>	vertices;
};
int current = 0;

std::vector<struct LineInfo> lines;

GLuint frameBuffer = 0;
GLuint texColorBuffer;
int working_width = 1980;
int working_height = 1080;

unsigned int scrrect_VBO = 0;
unsigned int  scrrect_VAO = 0;
float ratioX = 1.0f;
float ratioY = 1.0f;

void LoadTexture(const char * fname, GLuint* texture)
{
	int width = 0, height = 0, nrChannels;
	unsigned char* data = stbi_load(fname, &width, &height, &nrChannels, 0);
	GLuint format;

	if (nrChannels == 3) {
		format = GL_RGB;
	}
	else if (nrChannels == 4) {
		format = GL_RGBA;
	}

	if (data)
	{

		// Create Texture
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		// Set Texture wrap and filter modes
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	stbi_image_free(data);
}

void BuildFrameBuffer()
{
	// 1. create frame buffer
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// 2. create a blank texture which will contain the RGB output of our shader.
	// data is set to NULL
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


	shader_FB.Init();
	shader_FB.LoadVertexShaderFromFile("./shaders/basic_vshader_fb.txt");
	shader_FB.LoadFragmentShaderFromFile("./shaders/basic_fshader_fb.txt");
	shader_FB.Build();

	ratioX = working_width/(float)SCR_WIDTH;
	ratioY = working_height/ (float)SCR_HEIGHT;
}

int main(int argc, char* argv[])
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

	BuildFrameBuffer();

	// render loop
	// -----------
	double lastTime = glfwGetTime(), timer = lastTime;
	double elapsed = 0, nowTime = 0;

	
	GLShaderFx textureShader;
	textureShader.Init();
	textureShader.LoadVertexShaderFromFile("./shaders/sprite2D_vshader");
	textureShader.LoadFragmentShaderFromFile("./shaders/sprite2D_fshader");
	textureShader.Build();

	error = glGetError();

	unsigned int texture1 = 0;

	if (argc > 1) {
	
		LoadTexture(argv[1], &texture1);
	}

	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	GLuint _quadVAO = 0;
	GLuint _VBO = 0;

#ifndef OPENGLES2
	glGenVertexArrays(1, &_quadVAO);
	glBindVertexArray(_quadVAO);
#endif

	glGenBuffers(1, &_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifndef OPENGLES2
	glBindVertexArray(0);
#endif

	int x = 0, y = 0, width = 500, height = 500;
	GLint textureID = 0;

	if (argc >= 3)
	{
		width = atoi(argv[2]);
		height = atoi(argv[3]);
	}

	x = (working_width / 2) - (width / 2);
	y = (working_height / 2) - (height / 2);

	while (!glfwWindowShouldClose(window))
	{
		// - Measure time
		nowTime = glfwGetTime();
		elapsed = (nowTime - lastTime) / 1000.0f;
		lastTime = nowTime;

		// input
		// -----
		processInput(window);

		// deploy frame buffer
		// Render to our frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, working_width, working_height);		// use the entire texture,
																// this means that use the dimension set as our total 
																// display area
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (texture1) {
#if 1
			// Prepare transformations
			glm::mat4 model = glm::mat4(1.0f);	//initialize to identity

			//rotate matrix
			glm::mat4 rotMat = glm::mat4(1.0f);
			glm::mat4 transMat = glm::mat4(1.0f);
			transMat = glm::translate(transMat, glm::vec3(x, y, 0.0f));

			//Resize to current scale
			glm::mat4 scaleMat = glm::mat4(1.0f);
			scaleMat = glm::scale(scaleMat, glm::vec3(width, height, 1.0f));

			textureShader.Use();

			int projmtx = textureShader.GetUniformLocation("projection");
			textureShader.SetUniformMatrix(projmtx, projection);

			int modelmtx = textureShader.GetUniformLocation("model");
			model = transMat * scaleMat * rotMat;
			textureShader.SetUniformMatrix(modelmtx, model);

			textureID = textureShader.GetUniformLocation("gSampler");
			glUniform1i(textureID, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef OPENGLES2
			glBindVertexArray(_quadVAO);
#else
			glBindBuffer(GL_ARRAY_BUFFER, _VBO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

#endif
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifndef OPENGLES2
			glBindVertexArray(0);
#endif

			glBlendFunc(GL_ONE, GL_ONE);
			glDisable(GL_BLEND);
#endif
		}

#if 1
	
		shader.Use();
		modelmtx = shader.GetUniformLocation("model");
		projmtx = shader.GetUniformLocation("projection");
		projection = glm::ortho(0.0f, (float)working_width, (float)working_height, 0.0f, -1.0f, 1.0f);

		modelmat = glm::mat4(1.0f);	//initialize to identity
		shader.SetUniformMatrix(modelmtx, modelmat);
		shader.SetUniformMatrix(projmtx, projection);

		glLineWidth(20.0f);
#if 1
		for (int i = 0; i < lines.size(); i++) {
			unsigned int VBO = 0;
			unsigned int VAO = 0;

			struct LineInfo line = lines.at(i);
			//if (line.vertices.size() < 4) continue;

			glGenBuffers(1, &VBO);
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, line.vertices.size() * sizeof(float),			// Must be dynamic buffer
				line.vertices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);


			glDrawArrays(GL_LINE_STRIP, 0, line.vertices.size()/2);  // primitive type, first index, count

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glDeleteBuffers(1, &VBO);

		}
#endif
		glLineWidth(1.0f);

		// Restore frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// activate the texture unit 0 first before binding texture
		shader_FB.Use();
		textureID = shader_FB.GetUniformLocation("gSampler");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);	// bind the render target texture

		glBindVertexArray(scrrect_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
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
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {

#if 0	// N$ 
		// dump coordinates here
		for (int i = 0; i < lines.size(); i++) {
			struct LineInfo line = lines.at(i);
			printf("\n********************************************************");
			printf("\nPath %d", i);
			for (int v = 0; v < line.vertices.size();v+=2) {
				printf("\n path.push_back(Point2D(%.2f, %2.f));", line.vertices.at(v), line.vertices.at(v + 1));
			}
		}

#else // P$
		printf("\n**********************************************************");
		printf("\nSTART");

		for (int i = 0; i < lines.size(); i++) {
			struct LineInfo line = lines.at(i);
			printf("\nLine %d", i);
			for (int v = 0; v < line.vertices.size();v += 2) {
				printf("\n points.push_back(Point(%.2f, %2.f, %d));", line.vertices.at(v), line.vertices.at(v + 1), i);
			}
		}
		printf("\nEND");

#endif

	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		lines.clear();
		current = 0;

		printf("\n***************C L E A R E D********************************");
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		// 1.convert opengl coordinates to point2d
		// single gestures detection only for now
		// this demo proves that loaded in opengl or loaded by author can be detected.
		if (lines.size() < 0) return;
	
#if 0	// N$ 	
		
		struct LineInfo line = lines.at(0);
		Path2D path;
		
		for (int v = 0; v < line.vertices.size();v += 2) {
			path.push_back(Point2D(line.vertices.at(v), line.vertices.at(v + 1)));
		}
	
		GeometricRecognizer g;
		g.loadTemplates();

		// Sub-list of templates to search inside, should end by a "0"
		vector<string> gesturesList;
		gesturesList.push_back("Arrow");
		gesturesList.push_back("sample1");
		gesturesList.push_back("Circle");
		//gesturesList.push_back("Star");
		//gesturesList.push_back("Triangle");
		g.activateTemplates(gesturesList);

		RecognitionResult r = g.recognize(path);
		cout << "Recognized gesture: " << r.name << endl;
		cout << "1$ Score:" << r.score << endl;
#else
		std::vector<Gesture> trainingSet;
		trainingSet.push_back(GetTGestures());
		trainingSet.push_back(GetSGestures());
		trainingSet.push_back(GetS11_1());
		trainingSet.push_back(GetS12_1());

		std::vector<Point> points;
		for (int yng = 0; yng < lines.size(); yng++) {
			struct LineInfo line = lines.at(yng);
			for (int v = 0; v < line.vertices.size();v += 2) {
				points.push_back(Point(line.vertices.at(v), line.vertices.at(v + 1), yng));
			}	
		}

		Gesture candidate(points);
		float distance = 0.0f;
		std::string gestureClass = PointCloudRecognizer::Classify(candidate, trainingSet, &distance);
		printf("\n Classified = %s", gestureClass.c_str());
#endif

	}
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
			line->vertices.push_back(xpos * ratioX);
			line->vertices.push_back(ypos* ratioY);

			button_down = true;
			
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
		line->vertices.push_back(xpos * ratioX);
		line->vertices.push_back(ypos* ratioY);
	}
}

// P$ sample gestures
Gesture GetTGestures()
{
	std::vector<Point> Tgesture;

	Tgesture.push_back(Point(60, 13, 0));
	Tgesture.push_back(Point(46, 14, 0));
	Tgesture.push_back(Point(51, 14, 0));
	Tgesture.push_back(Point(57, 14, 0));
	Tgesture.push_back(Point(61, 14, 0));
	Tgesture.push_back(Point(65, 13, 0));
	Tgesture.push_back(Point(70, 13, 0));
	Tgesture.push_back(Point(76, 13, 0));
	Tgesture.push_back(Point(81, 12, 0));
	Tgesture.push_back(Point(88, 12, 0));
	Tgesture.push_back(Point(94, 11, 0));
	Tgesture.push_back(Point(101, 11, 0));
	Tgesture.push_back(Point(108, 10, 0));
	Tgesture.push_back(Point(114, 10, 0));
	Tgesture.push_back(Point(121, 9, 0));
	Tgesture.push_back(Point(126, 9, 0));
	Tgesture.push_back(Point(132, 8, 0));
	Tgesture.push_back(Point(137, 8, 0));
	Tgesture.push_back(Point(141, 8, 0));
	Tgesture.push_back(Point(145, 7, 0));
	Tgesture.push_back(Point(148, 7, 0));
	Tgesture.push_back(Point(150, 7, 0));
	Tgesture.push_back(Point(152, 7, 0));
	Tgesture.push_back(Point(153, 7, 0));
	Tgesture.push_back(Point(153, 7, 0));

	Tgesture.push_back(Point(106, 15, 1));
	Tgesture.push_back(Point(101, 19, 1));
	Tgesture.push_back(Point(104, 43, 1));
	Tgesture.push_back(Point(104, 51, 1));
	Tgesture.push_back(Point(104, 55, 1));
	Tgesture.push_back(Point(104, 59, 1));
	Tgesture.push_back(Point(104, 64, 1));
	Tgesture.push_back(Point(104, 68, 1));
	Tgesture.push_back(Point(104, 73, 1));
	Tgesture.push_back(Point(104, 77, 1));
	Tgesture.push_back(Point(104, 82, 1));
	Tgesture.push_back(Point(104, 86, 1));
	Tgesture.push_back(Point(104, 91, 1));
	Tgesture.push_back(Point(103, 95, 1));
	Tgesture.push_back(Point(103, 99, 1));
	Tgesture.push_back(Point(102, 104, 1));
	Tgesture.push_back(Point(102, 108, 1));
	Tgesture.push_back(Point(102, 112, 1));
	Tgesture.push_back(Point(101, 116, 1));
	Tgesture.push_back(Point(101, 120, 1));
	Tgesture.push_back(Point(100, 124, 1));
	Tgesture.push_back(Point(100, 127, 1));
	Tgesture.push_back(Point(100, 131, 1));
	Tgesture.push_back(Point(100, 134, 1));
	Tgesture.push_back(Point(99, 137, 1));
	Tgesture.push_back(Point(99, 139, 1));
	Tgesture.push_back(Point(99, 142, 1));
	Tgesture.push_back(Point(99, 144, 1));
	Tgesture.push_back(Point(98, 146, 1));
	Tgesture.push_back(Point(98, 147, 1));
	Tgesture.push_back(Point(98, 147, 1));

	return Gesture(Tgesture, "T");
}

Gesture GetSGestures()
{
	std::vector<Point> points;

	points.push_back(Point(1123.65, 211, 0));
	points.push_back(Point(1123.65, 209, 0));
	points.push_back(Point(1096.42, 205, 0));
	points.push_back(Point(1084.05, 203, 0));
	points.push_back(Point(1066.72, 200, 0));
	points.push_back(Point(1046.92, 196, 0));
	points.push_back(Point(1022.17, 194, 0));
	points.push_back(Point(994.95, 191, 0));
	points.push_back(Point(965.25, 189, 0));
	points.push_back(Point(947.92, 189, 0));
	points.push_back(Point(930.60, 189, 0));
	points.push_back(Point(913.27, 191, 0));
	points.push_back(Point(858.82, 209, 0));
	points.push_back(Point(853.87, 211, 0));
	points.push_back(Point(851.40, 218, 0));
	points.push_back(Point(848.92, 223, 0));
	points.push_back(Point(846.45, 230, 0));
	points.push_back(Point(843.97, 239, 0));
	points.push_back(Point(841.50, 250, 0));
	points.push_back(Point(836.55, 266, 0));
	points.push_back(Point(836.55, 268, 0));
	points.push_back(Point(836.55, 270, 0));
	points.push_back(Point(836.55, 275, 0));
	points.push_back(Point(843.97, 284, 0));
	points.push_back(Point(851.40, 292, 0));
	points.push_back(Point(863.77, 301, 0));
	points.push_back(Point(873.67, 306, 0));
	points.push_back(Point(888.52, 315, 0));
	points.push_back(Point(918.22, 331, 0));
	points.push_back(Point(930.60, 337, 0));
	points.push_back(Point(945.45, 346, 0));
	points.push_back(Point(957.82, 355, 0));
	points.push_back(Point(972.67, 365, 0));
	points.push_back(Point(987.52, 376, 0));
	points.push_back(Point(1004.85, 387, 0));
	points.push_back(Point(1017.22, 401, 0));
	points.push_back(Point(1034.55, 414, 0));
	points.push_back(Point(1079.10, 450, 0));
	points.push_back(Point(1089.00, 464, 0));
	points.push_back(Point(1098.90, 477, 0));
	points.push_back(Point(1108.80, 491, 0));
	points.push_back(Point(1111.27, 500, 0));
	points.push_back(Point(1113.75, 513, 0));
	points.push_back(Point(1113.75, 526, 0));
	points.push_back(Point(1113.75, 536, 0));
	points.push_back(Point(1113.75, 549, 0));
	points.push_back(Point(1113.75, 558, 0));
	points.push_back(Point(1103.85, 569, 0));
	points.push_back(Point(1096.42, 580, 0));
	points.push_back(Point(1081.57, 594, 0));
	points.push_back(Point(1066.72, 608, 0));
	points.push_back(Point(1054.35, 619, 0));
	points.push_back(Point(1039.50, 634, 0));
	points.push_back(Point(1022.17, 641, 0));
	points.push_back(Point(1012.27, 650, 0));
	points.push_back(Point(955.35, 673, 0));
	points.push_back(Point(925.65, 680, 0));
	points.push_back(Point(898.42, 682, 0));
	points.push_back(Point(873.67, 684, 0));
	points.push_back(Point(846.45, 684, 0));
	points.push_back(Point(829.12, 682, 0));
	points.push_back(Point(811.80, 679, 0));
	points.push_back(Point(792.00, 671, 0));
	points.push_back(Point(774.67, 664, 0));
	points.push_back(Point(754.88, 655, 0));
	points.push_back(Point(697.95, 623, 0));
	points.push_back(Point(690.52, 621, 0));
	points.push_back(Point(688.05, 617, 0));
	points.push_back(Point(685.57, 616, 0));
	points.push_back(Point(688.05, 616, 0));
	points.push_back(Point(690.52, 616, 0));
	points.push_back(Point(693.00, 616, 0));
	points.push_back(Point(695.47, 616, 0));

	return Gesture(points, "S");
}

Gesture GetS11_1()
{
	std::vector<Point> points;

	//	Line 0
	points.push_back(Point(1618.65, 394, 0));
	points.push_back(Point(1618.65, 394, 0));
	points.push_back(Point(1638.45, 409, 0));
	points.push_back(Point(1655.77, 423, 0));
	points.push_back(Point(1668.15, 434, 0));
	points.push_back(Point(1683.00, 448, 0));
	points.push_back(Point(1690.42, 455, 0));
	points.push_back(Point(1697.85, 463, 0));
	points.push_back(Point(1702.80, 466, 0));
	points.push_back(Point(1707.75, 470, 0));
	points.push_back(Point(1710.22, 472, 0));
	points.push_back(Point(1712.70, 473, 0));
	points.push_back(Point(1715.17, 477, 0));
	points.push_back(Point(1717.65, 479, 0));
	points.push_back(Point(1722.60, 482, 0));
	points.push_back(Point(1725.07, 486, 0));
	points.push_back(Point(1725.07, 488, 0));
	points.push_back(Point(1727.55, 491, 0));
	points.push_back(Point(1730.02, 493, 0));
	points.push_back(Point(1732.50, 493, 0));
	points.push_back(Point(1732.50, 495, 0));
	points.push_back(Point(1730.02, 495, 0));
	points.push_back(Point(1727.55, 491, 0));
	
	//Line 1
	points.push_back(Point(1678.05, 308, 1));
	points.push_back(Point(1678.05, 308, 1));
	points.push_back(Point(1707.75, 331, 1));
	points.push_back(Point(1732.50, 353, 1));
	points.push_back(Point(1749.82, 373, 1));
	points.push_back(Point(1762.20, 387, 1));
	points.push_back(Point(1779.52, 403, 1));
	points.push_back(Point(1786.95, 412, 1));
	points.push_back(Point(1796.85, 421, 1));
	points.push_back(Point(1804.27, 428, 1));
	points.push_back(Point(1809.22, 434, 1));
	points.push_back(Point(1814.17, 437, 1));
	points.push_back(Point(1821.60, 443, 1));
	points.push_back(Point(1824.07, 446, 1));
	points.push_back(Point(1826.55, 450, 1));
	points.push_back(Point(1831.50, 454, 1));
	points.push_back(Point(1833.97, 457, 1));
	points.push_back(Point(1836.45, 461, 1));
	points.push_back(Point(1841.40, 466, 1));
	points.push_back(Point(1843.87, 468, 1));
	points.push_back(Point(1846.35, 472, 1));
	points.push_back(Point(1848.82, 473, 1));
	points.push_back(Point(1851.30, 477, 1));
	points.push_back(Point(1856.25, 479, 1));
	points.push_back(Point(1858.72, 482, 1));
	points.push_back(Point(1861.20, 484, 1));
	points.push_back(Point(1861.20, 486, 1));
	points.push_back(Point(1856.25, 482, 1));
	points.push_back(Point(1856.25, 482, 1));
	points.push_back(Point(1851.30, 477, 1));
	points.push_back(Point(1843.87, 468, 1));
	
	//Line 2
	points.push_back(Point(1809.22, 272, 2));
	points.push_back(Point(1809.22, 272, 2));
	points.push_back(Point(1833.97, 295, 2));
	points.push_back(Point(1853.77, 315, 2));
	points.push_back(Point(1868.62, 331, 2));
	points.push_back(Point(1883.47, 349, 2));
	points.push_back(Point(1890.90, 356, 2));
	points.push_back(Point(1895.85, 362, 2));
	points.push_back(Point(1900.80, 365, 2));
	points.push_back(Point(1903.27, 367, 2));
	points.push_back(Point(1905.75, 371, 2));
	points.push_back(Point(1910.70, 374, 2));
	points.push_back(Point(1910.70, 376, 2));
	points.push_back(Point(1913.17, 378, 2));
	points.push_back(Point(1915.65, 380, 2));
	points.push_back(Point(1918.12, 382, 2));
	points.push_back(Point(1920.60, 383, 2));
	points.push_back(Point(1923.07, 387, 2));
	points.push_back(Point(1925.55, 391, 2));
	points.push_back(Point(1925.55, 392, 2));
	points.push_back(Point(1928.02, 394, 2));
	points.push_back(Point(1930.50, 396, 2));
	points.push_back(Point(1932.97, 398, 2));
	points.push_back(Point(1935.45, 400, 2));
	points.push_back(Point(1935.45, 401, 2));
	points.push_back(Point(1937.92, 401, 2));
	points.push_back(Point(1937.92, 403, 2));
	points.push_back(Point(1937.92, 405, 2));
	points.push_back(Point(1940.40, 405, 2));
	points.push_back(Point(1940.40, 407, 2));
	points.push_back(Point(1937.92, 405, 2));
	points.push_back(Point(1932.97, 403, 2));
	
	return Gesture(points, "S11");
}

Gesture GetS12_1()
{
	std::vector<Point> points;
	
	//	Line 0
	points.push_back(Point(1247.40, 281, 0));
	points.push_back(Point(1247.40, 281, 0));
	points.push_back(Point(1259.77, 281, 0));
	points.push_back(Point(1269.67, 281, 0));
	points.push_back(Point(1287.00, 281, 0));
	points.push_back(Point(1299.38, 281, 0));
	points.push_back(Point(1311.75, 281, 0));
	points.push_back(Point(1321.65, 281, 0));
	points.push_back(Point(1334.02, 281, 0));
	points.push_back(Point(1343.92, 281, 0));
	points.push_back(Point(1361.25, 281, 0));
	points.push_back(Point(1371.15, 281, 0));
	points.push_back(Point(1381.05, 281, 0));
	points.push_back(Point(1390.95, 281, 0));
	points.push_back(Point(1400.85, 281, 0));
	points.push_back(Point(1413.22, 281, 0));
	points.push_back(Point(1428.07, 281, 0));
	points.push_back(Point(1437.97, 281, 0));
	points.push_back(Point(1447.88, 279, 0));
	points.push_back(Point(1457.77, 279, 0));
	points.push_back(Point(1470.15, 279, 0));
	points.push_back(Point(1482.52, 277, 0));
	points.push_back(Point(1492.42, 277, 0));
	points.push_back(Point(1499.85, 277, 0));
	points.push_back(Point(1507.27, 277, 0));
	points.push_back(Point(1514.70, 277, 0));
	points.push_back(Point(1522.13, 277, 0));
	points.push_back(Point(1532.02, 277, 0));
	points.push_back(Point(1539.45, 277, 0));
	points.push_back(Point(1544.40, 277, 0));
	points.push_back(Point(1551.82, 277, 0));
	points.push_back(Point(1556.77, 277, 0));
	points.push_back(Point(1564.20, 277, 0));
	points.push_back(Point(1574.10, 277, 0));
	points.push_back(Point(1581.52, 277, 0));
	points.push_back(Point(1586.47, 277, 0));
	points.push_back(Point(1593.90, 277, 0));
	points.push_back(Point(1603.80, 277, 0));
	points.push_back(Point(1611.22, 277, 0));
	points.push_back(Point(1623.60, 275, 0));
	points.push_back(Point(1633.50, 275, 0));
	points.push_back(Point(1638.45, 275, 0));
	points.push_back(Point(1640.92, 274, 0));
	points.push_back(Point(1643.40, 274, 0));
	
	//Line 1
	points.push_back(Point(1437.97, 268, 1));
	points.push_back(Point(1437.97, 268, 1));
	points.push_back(Point(1440.45, 270, 1));
	points.push_back(Point(1442.92, 274, 1));
	points.push_back(Point(1447.88, 277, 1));
	points.push_back(Point(1450.35, 281, 1));
	points.push_back(Point(1455.30, 284, 1));
	points.push_back(Point(1457.77, 286, 1));
	points.push_back(Point(1460.25, 288, 1));
	points.push_back(Point(1462.72, 290, 1));
	points.push_back(Point(1467.67, 293, 1));
	points.push_back(Point(1470.15, 297, 1));
	points.push_back(Point(1472.63, 299, 1));
	points.push_back(Point(1475.10, 301, 1));
	points.push_back(Point(1477.57, 302, 1));
	points.push_back(Point(1480.05, 302, 1));
	points.push_back(Point(1482.52, 306, 1));
	points.push_back(Point(1482.52, 308, 1));
	points.push_back(Point(1485.00, 310, 1));
	points.push_back(Point(1487.47, 311, 1));
	points.push_back(Point(1487.47, 313, 1));
	points.push_back(Point(1489.95, 315, 1));
	points.push_back(Point(1492.42, 317, 1));
	points.push_back(Point(1494.90, 319, 1));
	points.push_back(Point(1494.90, 320, 1));
	points.push_back(Point(1497.38, 322, 1));
	points.push_back(Point(1499.85, 326, 1));
	points.push_back(Point(1502.32, 328, 1));
	points.push_back(Point(1502.32, 331, 1));
	points.push_back(Point(1504.80, 331, 1));
	points.push_back(Point(1507.27, 333, 1));
	points.push_back(Point(1507.27, 335, 1));
	points.push_back(Point(1509.75, 338, 1));
	points.push_back(Point(1512.22, 342, 1));
	points.push_back(Point(1512.22, 346, 1));
	points.push_back(Point(1514.70, 347, 1));
	points.push_back(Point(1514.70, 351, 1));
	points.push_back(Point(1517.17, 353, 1));
	points.push_back(Point(1517.17, 356, 1));
	points.push_back(Point(1519.65, 360, 1));
	points.push_back(Point(1519.65, 364, 1));
	points.push_back(Point(1522.13, 365, 1));
	points.push_back(Point(1522.13, 369, 1));
	points.push_back(Point(1522.13, 373, 1));
	points.push_back(Point(1522.13, 376, 1));
	points.push_back(Point(1524.60, 382, 1));
	points.push_back(Point(1524.60, 385, 1));
	points.push_back(Point(1527.07, 391, 1));
	points.push_back(Point(1527.07, 394, 1));
	points.push_back(Point(1527.07, 400, 1));
	points.push_back(Point(1527.07, 403, 1));
	points.push_back(Point(1529.55, 410, 1));
	points.push_back(Point(1529.55, 414, 1));
	points.push_back(Point(1529.55, 419, 1));
	points.push_back(Point(1529.55, 423, 1));
	points.push_back(Point(1529.55, 427, 1));
	points.push_back(Point(1529.55, 430, 1));
	points.push_back(Point(1529.55, 436, 1));
	points.push_back(Point(1529.55, 441, 1));
	points.push_back(Point(1529.55, 445, 1));
	points.push_back(Point(1529.55, 448, 1));
	points.push_back(Point(1529.55, 454, 1));
	points.push_back(Point(1529.55, 459, 1));
	points.push_back(Point(1527.07, 463, 1));
	points.push_back(Point(1524.60, 466, 1));
	points.push_back(Point(1524.60, 470, 1));
	points.push_back(Point(1522.13, 473, 1));
	points.push_back(Point(1522.13, 477, 1));
	points.push_back(Point(1519.65, 482, 1));
	points.push_back(Point(1517.17, 486, 1));
	points.push_back(Point(1514.70, 490, 1));
	points.push_back(Point(1512.22, 493, 1));
	points.push_back(Point(1509.75, 497, 1));
	points.push_back(Point(1504.80, 502, 1));
	points.push_back(Point(1499.85, 508, 1));
	points.push_back(Point(1497.38, 511, 1));
	points.push_back(Point(1494.90, 515, 1));
	points.push_back(Point(1489.95, 518, 1));
	points.push_back(Point(1485.00, 522, 1));
	points.push_back(Point(1480.05, 527, 1));
	points.push_back(Point(1475.10, 529, 1));
	points.push_back(Point(1470.15, 533, 1));
	points.push_back(Point(1467.67, 535, 1));
	points.push_back(Point(1462.72, 536, 1));
	points.push_back(Point(1457.77, 540, 1));
	points.push_back(Point(1452.82, 542, 1));
	points.push_back(Point(1447.88, 544, 1));
	points.push_back(Point(1442.92, 544, 1));
	points.push_back(Point(1440.45, 545, 1));
	points.push_back(Point(1435.50, 547, 1));
	points.push_back(Point(1433.02, 547, 1));
	points.push_back(Point(1428.07, 549, 1));
	points.push_back(Point(1420.65, 551, 1));
	points.push_back(Point(1418.17, 551, 1));
	points.push_back(Point(1413.22, 553, 1));
	points.push_back(Point(1410.75, 554, 1));
	points.push_back(Point(1405.80, 554, 1));
	points.push_back(Point(1403.32, 554, 1));
	points.push_back(Point(1398.38, 556, 1));
	points.push_back(Point(1395.90, 556, 1));
	points.push_back(Point(1390.95, 556, 1));
	points.push_back(Point(1388.47, 558, 1));
	points.push_back(Point(1386.00, 558, 1));
	points.push_back(Point(1381.05, 558, 1));
	points.push_back(Point(1378.57, 558, 1));
	points.push_back(Point(1373.63, 558, 1));
	points.push_back(Point(1371.15, 558, 1));
	points.push_back(Point(1368.67, 558, 1));
	points.push_back(Point(1366.20, 558, 1));
	points.push_back(Point(1361.25, 558, 1));
	points.push_back(Point(1358.77, 558, 1));
	points.push_back(Point(1356.30, 558, 1));
	points.push_back(Point(1351.35, 558, 1));
	points.push_back(Point(1348.88, 556, 1));
	points.push_back(Point(1346.40, 556, 1));
	points.push_back(Point(1341.45, 554, 1));
	points.push_back(Point(1336.50, 553, 1));
	points.push_back(Point(1334.02, 553, 1));
	points.push_back(Point(1331.55, 553, 1));
	points.push_back(Point(1326.60, 551, 1));
	points.push_back(Point(1324.13, 549, 1));
	points.push_back(Point(1321.65, 547, 1));
	points.push_back(Point(1319.17, 545, 1));
	points.push_back(Point(1319.17, 544, 1));
	points.push_back(Point(1316.70, 542, 1));
	points.push_back(Point(1311.75, 538, 1));
	points.push_back(Point(1309.27, 536, 1));
	points.push_back(Point(1306.80, 535, 1));
	points.push_back(Point(1304.32, 533, 1));
	points.push_back(Point(1304.32, 531, 1));
	points.push_back(Point(1301.85, 529, 1));
	points.push_back(Point(1299.38, 527, 1));
	points.push_back(Point(1299.38, 526, 1));
	points.push_back(Point(1296.90, 522, 1));
	points.push_back(Point(1296.90, 520, 1));
	points.push_back(Point(1296.90, 518, 1));
	points.push_back(Point(1296.90, 517, 1));
	points.push_back(Point(1294.42, 513, 1));
	points.push_back(Point(1294.42, 511, 1));
	points.push_back(Point(1291.95, 509, 1));
	points.push_back(Point(1291.95, 506, 1));
	points.push_back(Point(1289.47, 506, 1));
	points.push_back(Point(1289.47, 502, 1));
	points.push_back(Point(1287.00, 499, 1));
	points.push_back(Point(1287.00, 497, 1));
	points.push_back(Point(1287.00, 493, 1));
	points.push_back(Point(1284.52, 491, 1));
	points.push_back(Point(1284.52, 488, 1));
	points.push_back(Point(1284.52, 486, 1));
	points.push_back(Point(1282.05, 481, 1));
	points.push_back(Point(1282.05, 479, 1));
	points.push_back(Point(1282.05, 477, 1));
	points.push_back(Point(1282.05, 475, 1));
	points.push_back(Point(1282.05, 473, 1));
	points.push_back(Point(1282.05, 472, 1));
	points.push_back(Point(1282.05, 468, 1));
	points.push_back(Point(1282.05, 466, 1));
	points.push_back(Point(1282.05, 463, 1));
	points.push_back(Point(1284.52, 461, 1));
	points.push_back(Point(1287.00, 457, 1));
	points.push_back(Point(1289.47, 455, 1));
	points.push_back(Point(1291.95, 454, 1));
	points.push_back(Point(1294.42, 452, 1));
	points.push_back(Point(1296.90, 450, 1));
	points.push_back(Point(1299.38, 448, 1));
	points.push_back(Point(1301.85, 446, 1));
	points.push_back(Point(1304.32, 445, 1));
	points.push_back(Point(1306.80, 443, 1));
	points.push_back(Point(1309.27, 441, 1));
	points.push_back(Point(1311.75, 441, 1));
	points.push_back(Point(1311.75, 439, 1));
	points.push_back(Point(1316.70, 439, 1));
	points.push_back(Point(1321.65, 437, 1));
	points.push_back(Point(1324.13, 436, 1));
	points.push_back(Point(1329.07, 436, 1));
	//Line 2
	return Gesture(points, "S12");
}