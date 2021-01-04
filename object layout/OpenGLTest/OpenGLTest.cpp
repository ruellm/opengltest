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
#include "game/actor_builder.h"
#include "utility/utility.h"
#include "graphics/sprite2d.h"
#include "graphics/assimp_mesh_sprite.h"

#define SCR_WIDTH	960
#define SCR_HEIGHT	540

// settings
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

using namespace std;
using namespace ramo::effects;
using namespace ramo::graphics;

static std::vector<GameObject*> objectList;
 
extern void SetProjectionMatrix(glm::mat4 matrix);
extern glm::mat4 GetProjectionMatrix();

int main(int argc, char* argv[])
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR	, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR	, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE			, GLFW_OPENGL_CORE_PROFILE);

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
	Engine* engine = Engine::GetInstance();
	engine->GetGraphics()->Initialize();

	InitializeComponentSystem();

	glEnable(GL_DEPTH_TEST);
	float lasttime = 0;
	float angle = 0;

	AssimpMesh* mesh = new AssimpMesh();
	mesh->Load("models/F - Mana Stone/F - Mana Stone.fbx");
	
	GameObject object;
	object.SetMesh(mesh);
	object.SetPosition(glm::vec3(0, 0, 0));
	
	camera.LookAt(glm::vec3(0, 0, -50), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));

	GLShaderFx shader;
	shader.Init();
	shader.LoadVertexShaderFromFile("./shaders/skinning.vs");
	shader.LoadFragmentShaderFromFile("./shaders/color_alpha.fs");
	shader.Build();
	
	angle = 290.0f;
	while (!glfwWindowShouldClose(window))
	{
		
		float current_time = glfwGetTime();
		float elapsed = (current_time - lasttime);
		lasttime = current_time;

		// input
		// -----
		angle += 1.0f * elapsed;
		//printf("\n angle %f", angle);

		processInput(window);
		//float angleradians = glm::radians(angle);

		glStencilMask(0xFF); // enable writing to the stencil buffer, 
							// discovered 5-8-2019 when writing to stenci is disabled,
							// glClearColor and glClear will not work or clear the stencil buffer
							// thus maintaining the old values it has
							// glStencilMask  will affect glClear

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#if 1
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);

		glClearStencil(0);

		// 1st. render pass, draw objects as normal, writing to the stencil buffer
	   // --------------------------------------------------------------------
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should update the stencil buffer
		glStencilMask(0xFF); // enable writing to the stencil buffer
#endif
		object.SetScale(1, 1, 1);
		object.SetAngleY(angle);
		UpdateObject(elapsed, &object);
		DrawObject(&object);

#if 1
		// 2nd. render pass: now draw slightly scaled versions of the objects, this time disabling stencil writing.
		// Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
		// the objects' size differences, making it look like borders.
		// -----------------------------------------------------------------------------------------------------------------------------
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00); // disable writing to the stencil buffer
		glDisable(GL_DEPTH_TEST);
#endif

		shader.Use();
		int color = shader.GetUniformLocation("color");
		shader.SetUniformVector4(color, glm::vec4(1, 0, 0, 1));
		object.SetScale(1.1, 1.1, 1.1);
		//object.SetAngleY(0);
		UpdateObject(elapsed, &object);
		DrawMesh(&object, &shader);

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


}
