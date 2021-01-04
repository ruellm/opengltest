// OpenGLTest.cpp : Defines the entry point for the console application.
// Mini Effect Editor, created 3-29-2019
// Author: ruellm@yahoo.com

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "assimp.lib")

// WINSOCK required libraries
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "common.h"
#include <iostream>
#include <algorithm>  
#include <iostream>
#include <string>
#include <fstream>

//ramo engine
#include "adapter.h"
#include "engine/engine.h"
#include "engine/event.h"
#include "engine/graphics.h"
#include "utility/Tokenizer.h"
#include "utility/string_helper.h"
#include "scene/game_object.h"
#include "scene/camera.h"
#include "effects/particle_system.h"
#include "utility/string_helper.h"
#include "graphics/billboard.h"
#include "graphics/textured_rect3d.h"
#include "graphics/assimp_mesh.h"
#include "ComponentSystem.h"

#include "effects/particle_emitters.h"
#include "effects/zoom_rect_vfx.h"
#include "effects/healing_vfx.h"
#include "effects/buff_debuff_vfx.h"
#include "effects/magic_cast_vfx.h"

#define SCR_WIDTH	960
#define SCR_HEIGHT	540

// settings
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

void GenerateGrid(int cellcnt_w, int cellcnt_h);

using namespace std;
using namespace ramo::effects;
using namespace ramo::graphics;

static std::string strFile;
static std::vector<GameObject*> objectList;

static GLuint grid_vba = 0;
static GLuint grid_vbo = 0;
static int grid_vert_count = 0; 
static GLShaderFx grid_shader;


void CleanupObjects()
{
	for (int i = 0; i < objectList.size(); i++)
	{
		SAFE_DELETE(objectList.at(i));
	}
	objectList.clear();

}

int main(int argc, char* argv[])
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RAMO Mini VFX editor", NULL, NULL);
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
	Adapter_Initialize();

	glm::vec3 lightPosition = glm::vec3(-1.16024923, 28.2320156, -33.0286446);
	camera.LookAt(
		//glm::vec3(0, -10, -10),
		lightPosition,
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 0));
#if 0
	camera.LookAt(
		glm::vec3(0, 5, 5),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 0));
#endif
	InitializeComponentSystem();

	grid_shader.Init();
	grid_shader.Load("shaders/vshader_world", GL_VERTEX_SHADER);
	grid_shader.Load("shaders/fshader_world", GL_FRAGMENT_SHADER);
	grid_shader.Build();
	GenerateGrid(50, 50);

	glEnable(GL_DEPTH_TEST);
	float lasttime = 0;
	float angle = 0;

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE); //enable this
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	AssimpMesh* mesh = new AssimpMesh();
	mesh->Load("models/scene1/Cube-3 and Plane.fbx");
	GameObject* obj = new GameObject();
	obj->SetMesh(mesh);
	obj->SetPosition(glm::vec3(0, 0, 0));
	objectList.push_back(obj);

	glm::vec3 lightDirection = glm::vec3(0, -5.5, 5);// glm::vec3(1, 0, 0);

	// 1. Shadow map step 1 - setup a render target FBO
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	GLuint depthTexture=0;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glDrawBuffer(GL_NONE); // No color buffer is drawn to.
	glReadBuffer(GL_NONE);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return 1;

	// unbind current framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//2. step 2 setup projection and view matrix of light
	//glm::vec3 lightInvDir = -lightDirection;
	glm::vec3 lightDirNorm = glm::normalize(lightPosition);
	SetDirectional(-lightDirNorm);

	// Compute the MVP matrix from the light's point of view
	// the sunlight does not have a position and everything is rendered in parallel
	glm::mat4 depthProjectionMatrix = glm::ortho<float>(-30, 30, -30, 30, -30, 100); // glm::ortho<float>(-10, 10, -10, 10, -10, 20);
	glm::mat4 depthViewMatrix = glm::lookAt(lightPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 depthModelMatrix = glm::mat4(1.0);
	
	SetShadowMVP(depthViewMatrix, depthProjectionMatrix);

	GLShaderFx orthoShader;
	orthoShader.Init();
	orthoShader.Load("shaders/basic_vshader_clipspace.txt", GL_VERTEX_SHADER);
	orthoShader.Load("shaders/basic_fshader_texcoord.txt", GL_FRAGMENT_SHADER);
	orthoShader.Build();
	orthoShader.Use();

	FBORectangle rect;

	rect._position_attrib_id = orthoShader.GetAttributeLocation("position");
	rect._texture_attrib_id = orthoShader.GetAttributeLocation("textCoord");
	rect.Create();

	GLuint error = 0;
	error = glGetError();

	AssimpMesh* a1 = new AssimpMesh();
	a1->Load("models/a1/A1-3D.fbx");
	a1->AddAnimationByFrame(0, 48, 95, true);
	a1->SetAnimation(0);

	GameObject* avatar = new GameObject();
	avatar->SetMesh(a1);
	avatar->SetPosition(glm::vec3(0, 0, 0));
	objectList.push_back(avatar);

	while (!glfwWindowShouldClose(window))
	{
		
		float current_time = glfwGetTime();
		float elapsed = (current_time - lasttime);
		lasttime = current_time;

		camera.Update();

		for(int i = 0;i < objectList.size(); i++)
			UpdateObject(elapsed, objectList.at(i), glm::mat4(1.0f));

		// input
		// -----
		processInput(window);
#if 1
		// --- Bind framebuffer
		GLint backup_FBO = 0;
		int backupWidth = 0;
		int backupHeight = 0;
		// Render to our frame buffer
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &backup_FBO);

		GLint dims[4] = { 0 };
		glGetIntegerv(GL_VIEWPORT, dims);
		backupWidth = dims[2];
		backupHeight = dims[3];

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		error = glGetError();
		glViewport(0, 0, 1024, 1024);
		error = glGetError();
#endif
		// render
		// ------
	//	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		//...
		// Draw the GRID
#if 0
		grid_shader.Use();
		int model = grid_shader.GetUniformLocation("model");
		int projection = grid_shader.GetUniformLocation("projection");
		int view = grid_shader.GetUniformLocation("view");
		glm::mat4 world = glm::mat4(1.0f);
		glm::mat4 projectionMat = GetProjectionMatrix();
		grid_shader.SetUniformMatrix(model, world);
		grid_shader.SetUniformMatrix(projection, projectionMat);
		grid_shader.SetUniformMatrix(view, camera.GetViewMatrix());

		glBindVertexArray(grid_vba);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_LINES, 0, grid_vert_count);
		glDisableVertexAttribArray(0);
		glBindVertexArray(0);
#endif
		glCullFace(GL_FRONT);
		glEnable(GL_DEPTH_TEST); ////--- this took us whole day to debug!! enable depth test, since we disabled it drawing 2D!
		for (int i = 0; i < objectList.size(); i++)
			DrawObjectShadowMap(objectList.at(i));

		SetShadowTexture(depthTexture);

		glCullFace(GL_BACK); // don't forget to reset original culling face

#if 1	//Enable this for 3D testing
		glBindFramebuffer(GL_FRAMEBUFFER, backup_FBO);
		error = glGetError();
		glViewport(0, 0, backupWidth, backupHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < objectList.size(); i++)
			DrawObject(objectList.at(i));
#endif

#if 0	//This testing is for depth texture rendering
		error = glGetError();

		glBindFramebuffer(GL_FRAMEBUFFER, backup_FBO);
		error = glGetError();
		glViewport(0, 0, backupWidth, backupHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		orthoShader.Use();
		int textureID = orthoShader.GetUniformLocation("gSampler");
		glUniform1i(textureID, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);	// bind the render target texture

		glDisable(GL_DEPTH_TEST);  // disable since we are rendering in 2D

		rect._position_attrib_id = orthoShader.GetAttributeLocation("position");
		rect._texture_attrib_id = orthoShader.GetAttributeLocation("textCoord");
		rect.Draw();

		glBindTexture(GL_TEXTURE_2D, 0);
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	if (FramebufferName)
		glDeleteFramebuffers(1, &FramebufferName);
	
	if (depthTexture)
		glDeleteTextures(1, &depthTexture);

	CleanupObjects();
	glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	Engine::GetInstance()->GetGraphics()->Resize(width, height);
	
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
bool mbutton_down = false;
double lastx, lasty;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double x, y;

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS) {
			mbutton_down = true;

			glfwGetCursorPos(window, &lastx, &lasty);
		}

		else if (action == GLFW_RELEASE) {
			mbutton_down = false;
		}
	}

	int touchtype = 0;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		touchtype = EVENT_TYPE_TOUCH_DOWN;
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		touchtype = EVENT_TYPE_TOUCH_UP;

	glfwGetCursorPos(window, &x, &y);
	
	Engine::GetInstance()->OnTouchEvent((int)touchtype, (float)x, (float)y);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (mbutton_down) {
		// mouse move
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		double diffx = (xpos - lastx) * 0.2f;
		double diffy = (ypos - lasty) * 0.2f;

		camera.Yaw(-diffx);
		camera.Pitch(-diffy);

		lastx = xpos;
		lasty = ypos;

	}

	int touchtype = EVENT_TYPE_TOUCH_MOVE;
	Engine::GetInstance()->OnTouchEvent((int)touchtype, (float)xpos, (float)ypos);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float sensitivity = 0.009f;

	// -----------------------------------------------------

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.Walk(-sensitivity);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.Walk(sensitivity);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.Strafe(-sensitivity);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.Strafe(sensitivity);
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		camera.Fly(sensitivity);
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		camera.Fly(-sensitivity);
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		camera.Pitch(1);
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		camera.Pitch(-1);
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		camera.Yaw(1);
	}
	
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		camera.Yaw(-1);
	}

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		// Reload config
	}
}


void GenerateGrid(int cellcnt_w, int cellcnt_h)
{
	int horiz_cnt = cellcnt_h + 1;
	int vert_cnt = cellcnt_w + 1;
	float cellspace = 1;// 0.5f;
	float WIDE = cellspace * (float)cellcnt_w;
	float HEIGHT = cellspace * (float)cellcnt_h;

	grid_vert_count = (horiz_cnt * 2) + (vert_cnt * 2);

	glGenVertexArrays(1, &grid_vba);
	glBindVertexArray(grid_vba);

	glGenBuffers(1, &grid_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);

	//--------------------------------------------
	//Temporary Grid will be built in Y plane instead of Z.
	//Actual will be on Z Axis
	unsigned long size = (horiz_cnt * 6) + (vert_cnt * 6);
	float* buffer = new float[size];

	float xoffset = -WIDE / 2.0f;
	float yoffset = WIDE / 2.0f;
	int index = 0;

	for (int x = 0; x < horiz_cnt; x++) {

		buffer[index++] = xoffset;
		buffer[index++] = 0;
		buffer[index++] = yoffset;

		buffer[index++] = xoffset + WIDE;
		buffer[index++] = 0;
		buffer[index++] = yoffset;

		yoffset -= cellspace;
	}

	xoffset = -WIDE / 2;
	yoffset = WIDE / 2;
	for (int x = 0; x < vert_cnt; x++) {
		buffer[index++] = xoffset;
		buffer[index++] = 0;
		buffer[index++] = yoffset;

		buffer[index++] = xoffset;
		buffer[index++] = 0;
		buffer[index++] = yoffset - HEIGHT;

		xoffset += cellspace;
	}

	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), buffer, GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);


	//make sure to cleanup
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum error = glGetError();
	delete[] buffer;
}
