// OpenGLTest.cpp : Defines the entry point for the console application.
//

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
#include "game/game.h"

// For Debugging purposes
#include "states/battle_state.h"
#include "states/academy_state.h"
#include "states/map_state.h"
#include "states/astral_battlefield_state.h"
#include "unittest/unittest_1.h"

#define SCR_WIDTH	960
#define SCR_HEIGHT	540

// settings
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

bool fly_mode = false;
Camera* camera = NULL;
int z_value = 0;

using namespace std;

#define CONFIG_FILE "./config.txt"
struct EngineConfig
{
	float fov;
	bool fog;
	float fog_density;
	float fog_gradient;
	bool lighted;
	glm::vec3 light_position;
	int light_config;

};

// Desktop EDItor
bool ReadConfiguration(const char* szConfigFile)
{
	ifstream file(szConfigFile); //file just has some sentences
	if (!file) {
		cout << "unable to open file";
		return false;
	}
	string sent;
	EngineConfig config;
	while (getline(file, sent))
	{
		cout << sent << endl;
		
	
		Tokens tokens;
		Tokenize(sent, tokens, "=");

		if (tokens.at(0) == "fov")
		{
			config.fov = atof(tokens.at(1).c_str());
		}
		else if (tokens.at(0) == "fog")
		{
			config.fog = (tokens.at(1) == "true") ? true : false;
		}
		else if (tokens.at(0) == "fog_density")
		{
			config.fog_density = atof(tokens.at(1).c_str());
		}

		else if (tokens.at(0) == "fog_gradient")
		{
			config.fog_gradient = atof(tokens.at(1).c_str());
		}

		else if (tokens.at(0) == "light_position")
		{
			std::string params = trim(tokens.at(1));
			Tokens parameters;
			Tokenize(params, parameters, ",");
			float x, y, z;
			
			x = atof(parameters.at(0).c_str());
			y = atof(parameters.at(1).c_str());
			z = atof(parameters.at(2).c_str());

			config.light_position = glm::vec3(x, y, z);
		}
		else if (tokens.at(0) == "light_config")
		{
			if (tokens.at(1) == "all")
			{
				config.light_config = 2;
			}
			else if (tokens.at(1) == "terrain")
			{
				config.light_config = 1;
			}
			else if (tokens.at(1) == "off")
			{
				config.light_config = 0;
			}
		}
	}

	glm::mat4 projection = glm::perspective(glm::radians(config.fov), 4.0f / 3.0f, 1.0f, 100000.0f);
	Engine::GetGraphics()->SetProjectionMatrix(projection);

	Engine::GetGraphics()->GetFogConfiguration()->SetEnable(config.fog);
	Engine::GetGraphics()->GetFogConfiguration()->SetDensity(config.fog_density);
	Engine::GetGraphics()->GetFogConfiguration()->SetGradient(config.fog_gradient);

	if(Engine::GetGraphics()->GetPointLightConfiguration()->light_list.size() > 0)
		Engine::GetGraphics()->GetPointLightConfiguration()->light_list.at(0).position = config.light_position;

	Engine::GetGraphics()->GetPointLightConfiguration()->config = config.light_config;
}


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR	, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR	, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE			, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	std::string strVersion("RAMO ");
	strVersion = strVersion + std::string(GAME_VERSION);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, strVersion.c_str(), NULL, NULL);
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


	glEnable(GL_DEPTH_TEST);
	float lasttime = 0;
	float angle = 0;

#if 1
	Adapter_Initialize();
	Adapter_StartEngine();

	Engine* engine = Engine::GetInstance();
	if (engine == NULL) return 0;
	Engine::GetInstance()->GetGraphics()->Resize(SCR_WIDTH, SCR_HEIGHT);

#ifdef UNIT_TESTING
	UnitTest_01* state = (UnitTest_01*)Engine::GetInstance()->GetState(UNIT1_STATE);
	camera = state->GetCamera();
#else
	//BattleState* state = (BattleState*)Engine::GetInstance()->GetState(BATTLE_STATE);
	//AcademyState* state = (AcademyState*)Engine::GetInstance()->GetState(ACADEMY_STATE);
	AstralBattleFieldState* state = (AstralBattleFieldState*)Engine::GetInstance()->GetState(ASTRAL_BATTLE_FIELD_STATE);
	camera = state->GetCamera();
#endif
	
#endif

	// for desktop engine purposes only
	//ReadConfiguration(CONFIG_FILE);

	while (!glfwWindowShouldClose(window))
	{
		
		// input
		// -----
		processInput(window);

		// render
		// ------
		engine->Step();
		//...
		
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

	if (mbutton_down && fly_mode) {
		// mouse move
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		double diffx = (xpos - lastx) * 0.2f;
		double diffy = (ypos - lasty) * 0.2f;

		camera->Yaw(-diffx);
		camera->Pitch(-diffy);

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

	float sensitivity = 1;// 0.01f;

	// -----------------------------------------------------
	// EDITOR COMMANDS
	
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		fly_mode = !fly_mode;
		if(fly_mode)
			printf("\n EDIT MODE = ENABLED");
		else
			printf("\n EDIT MODE = DISABLED");
	}

	
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		printf("\n ********************************************");
		printf("\nCamera settings (Position): (%f, %f, %f)", 
			camera->GetPosition().x, 
			camera->GetPosition().y, 
			camera->GetPosition().z);

		glm::vec3 up = camera->GetUp();
		printf("\nCamera settings (up): (%f, %f, %f)",
			up.x,
			up.y,
			up.z);

		glm::vec3 look = camera->GetPosition() + camera->GetLook();
		printf("\nCamera settings (Lookat): (%f, %f, %f)",
			look.x,
			look.y,
			look.z);

		printf("\n Hex Z value offset: %d", z_value);

		printf("\n ********************************************");
	}

	if (!fly_mode) return;
	// -----------------------------------------------------

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera->Walk(-sensitivity);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera->Walk(sensitivity);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera->Strafe(-sensitivity);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera->Strafe(sensitivity);
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		camera->Fly(sensitivity);
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		camera->Fly(-sensitivity);
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		camera->Pitch(1);
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		camera->Pitch(-1);
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		camera->Yaw(1);
	}
	
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		camera->Yaw(-1);
	}

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		// Reload config
		ReadConfiguration(CONFIG_FILE);
	}
#if 0
	State* 
	float rotateSensitivity = 0.5f;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		townState->RotateCenter(-rotateSensitivity, 0);
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		townState->RotateCenter(rotateSensitivity, 0);
	}

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		townState->RotateCenter(-rotateSensitivity, 1);
	}

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		townState->RotateCenter(rotateSensitivity, 1);
	}
#endif

	MapState* state = (MapState*)Engine::GetInstance()->GetState(BATTLE_STATE);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		state->PinchZoom(1);
		//z_value++;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		state->PinchZoom(-1);
		//z_value--;
	}
}
