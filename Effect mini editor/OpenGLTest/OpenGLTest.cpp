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
#include "effects/particle_emitters.h"
#include "utility/string_helper.h"
#include "graphics/billboard.h"
#include "graphics/textured_rect3d.h"
#include "graphics/assimp_mesh.h"
#include "ComponentSystem.h"

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


int GetUpdateParameter(ifstream& file, AnimatePerFrameParam* param)
{
	string data;
	while (getline(file, data))
	{
		cout << data << endl;

		if (data.size() <= 0) continue;
		if (data.c_str()[0] == '#') continue;
		data = RemoveWhitespace(data);

		Tokens tokens;
		Tokenize(data, tokens, "=");
		std::string attribute = tokens.at(0);

		if (attribute == "update_end") break;

		else if (attribute == "alpha") {
			param->alpha = atof(tokens.at(1).c_str());
		}
		else if (attribute == "scaleX") {
			param->scaleX = atof(tokens.at(1).c_str());
		}
		else if (attribute == "scaleY") {
			param->scaleY = atof(tokens.at(1).c_str());
		}
		else if (attribute == "scaleZ") {
			param->scaleZ = atof(tokens.at(1).c_str());
		}
		else if (attribute == "rotationX") {
			param->rotX = atof(tokens.at(1).c_str());
		}
		else if (attribute == "rotationY") {
			param->rotY = atof(tokens.at(1).c_str());
		}
		else if (attribute == "rotationZ") {
			param->rotZ = atof(tokens.at(1).c_str());
		}
		else if (attribute == "positionX") {
			param->posX = atof(tokens.at(1).c_str());
		}
		else if (attribute == "positionY") {
			param->posY = atof(tokens.at(1).c_str());
		}
		else if (attribute == "positionZ") {
			param->posZ = atof(tokens.at(1).c_str());
		}
	}


	return 0;
}

int GetParticleParameter(ifstream& file)
{
	string data;
	ParticleSystem* particle = new ParticleSystem();
	float alpha = 1.0f;
	AnimatePerFrameParam param;

	while (getline(file, data))
	{
		cout << data << endl;

		if (data.size() <= 0) continue;
		if (data.c_str()[0] == '#') continue;

		data = RemoveWhitespace(data);

		Tokens tokens;
		Tokenize(data, tokens, "=");
		std::string attribute = tokens.at(0);

		if (attribute == "end") break;	
		else if (attribute == "filepath")
		{
			particle->LoadTexture(tokens.at(1).c_str());
		}
		else if (attribute == "position")
		{
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			float x, y, z;

			x = atof(parameters.at(0).c_str());
			y = atof(parameters.at(1).c_str());
			z = atof(parameters.at(2).c_str());

			particle->_position = glm::vec3(x, y, z);
		}
		else if (attribute == "start_size")
		{
			particle->_startSize = atof(tokens.at(1).c_str());
		}
		else if (attribute == "max_particles")
		{
			particle->_maxParticles = atof(tokens.at(1).c_str());
		}
		else if (attribute == "life_time")
		{
			particle->_lifeTime = atof(tokens.at(1).c_str());
		}
		else if (attribute == "start_speed")
		{
			particle->_startSpeed = atof(tokens.at(1).c_str());
		}
		else if (attribute == "shape")
		{
			if (tokens.at(1) == "cube")
			{
				particle->_emitter = new CubeEmitter();
			}
			else if (tokens.at(1) == "circle")
			{
				particle->_emitter = new CircleEmitter();
			}
		}
		
		else if (attribute == "shape.cube.thicknessX")
		{
			if (particle->_emitter == NULL) continue;
			((CubeEmitter*)particle->_emitter)->_thicknessX = atof(tokens.at(1).c_str());;
		}
		else if (attribute == "shape.cube.thicknessY")
		{
			if (particle->_emitter == NULL) continue;
			((CubeEmitter*)particle->_emitter)->_thicknessY = atof(tokens.at(1).c_str());;
		}
		else if (attribute == "shape.cube.thicknessZ")
		{
			if (particle->_emitter == NULL) continue;
			((CubeEmitter*)particle->_emitter)->_thicknessZ = atof(tokens.at(1).c_str());;
		}
		else if (attribute == "shape.circle.radius")
		{
			if (particle->_emitter == NULL) continue;
			((CircleEmitter*)particle->_emitter)->_radius = atof(tokens.at(1).c_str());;
		}
		else if (attribute == "shape.circle.is_edge_only")
		{
			if (particle->_emitter == NULL) continue;
			((CircleEmitter*)particle->_emitter)->_isEdgeOnly = (tokens.at(1)=="true")?true:false;
		}
		/*else if (attribute == "shape.bidirectional_velocity")
		{
			if (particle->_emitter == NULL) continue;
			//particle->_emitter->bidirectional = (tokens.at(1) == "true") ? true : false;

			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");

			particle->_emitter->bidirectional[0] = (parameters.at(0) == "true") ? true : false;
			particle->_emitter->bidirectional[1] = (parameters.at(1) == "true") ? true : false;
			particle->_emitter->bidirectional[2] = (parameters.at(2) == "true") ? true : false;

		}*/
		
		else if (attribute == "shape.velocity")
		{
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			float x, y, z;

			x = atof(parameters.at(0).c_str());
			y = atof(parameters.at(1).c_str());
			z = atof(parameters.at(2).c_str());
	
			particle->_emitter->velocity = glm::vec3(x, y, z);
		}

		else if (attribute == "start_color")
		{
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			float r, g, b, a;

			r = atof(parameters.at(0).c_str());
			g = atof(parameters.at(1).c_str());
			b = atof(parameters.at(2).c_str());
			a = atof(parameters.at(3).c_str());

			particle->_startColor = glm::vec4(r, g, b, a);
		}

		else if (attribute == "color_over_lifetime")
		{
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			float pct, r, g, b, a;

			pct = atof(parameters.at(0).c_str());
			r = atof(parameters.at(1).c_str());
			g = atof(parameters.at(2).c_str());
			b = atof(parameters.at(3).c_str());
			a = atof(parameters.at(4).c_str());

			particle->AddColorInterpolator(pct, glm::vec4(r, g, b, a));
		}
		else if (attribute == "size_over_lifetime")
		{
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			float pct, size;

			pct = atof(parameters.at(0).c_str());
			size = atof(parameters.at(1).c_str());

			particle->AddSizeInterpolator(pct, size);
		}

		else if (attribute == "blend_mode")
		{
			if (tokens.at(1) == "additive")
				particle->_blendMode = PARTICLE_BLENDMODE_ADDITIVE;
			else if (tokens.at(1) == "one")
				particle->_blendMode = PARTICLE_BLENDMODE_ONE;
		}
		else if (attribute == "update_start")
		{
			GetUpdateParameter(file, &param);
		}
		else if (attribute == "alpha") {
			alpha = atof(tokens.at(1).c_str());
		}
	}

	AnimatedGameObject* obj = new AnimatedGameObject();
	obj->_anim_param = param;
	obj->SetParticle(particle);
	obj->SetAlpha(alpha);
	objectList.push_back(obj);
	particle->Start();

	return 0;
}

int GetCustomMeshParameters(ifstream& file, int flag)
{
	string data;
	RectMesh* mesh = new RectMesh();
	std::string path("");
	int frame_width = 0;
	int frame_count = 1;
	int fps = 0;
	bool loop = false;
	float alpha = 1.0f;
	AnimatePerFrameParam param;
	float rotateX = 0, rotateY = 0, rotateZ = 0;

	if (flag == 0)
		mesh = new Billboard();
	else if (flag == 1)
		mesh = new TexturedRect3D();

	while (getline(file, data))
	{
		cout << data << endl;

		if (data.size() <= 0) continue;
		if (data.c_str()[0] == '#') continue;
		data = RemoveWhitespace(data);

		Tokens tokens;
		Tokenize(data, tokens, "=");
		std::string attribute = tokens.at(0);

		if (attribute == "end") break;
		else if (attribute == "filepath")
		{
			path = tokens.at(1);
		}
		else if (attribute == "dimension") {
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			float width, height;

			width = atof(parameters.at(0).c_str());
			height = atof(parameters.at(1).c_str());

			mesh->_scale = glm::vec2(width, height);
		}
		else if (attribute == "frame_width") {
			frame_width = atoi(tokens.at(1).c_str());
		}
		else if (attribute == "frame_count") {
			frame_count = atoi(tokens.at(1).c_str());
		}
		else if (attribute == "fps") {
			fps = atoi(tokens.at(1).c_str());
		}
		else if (attribute == "loop") {
			loop = (tokens.at(1) == "true") ? true : false;
		}
		else if (attribute == "position")
		{
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			float x, y, z;

			x = atof(parameters.at(0).c_str());
			y = atof(parameters.at(1).c_str());
			z = atof(parameters.at(2).c_str());

			mesh->_position = glm::vec3(x, y, z);
		}
		else if (attribute == "update_start")
		{
			GetUpdateParameter(file, &param);
		}
		else if (attribute == "alpha") {
			alpha = atof(tokens.at(1).c_str());
		}

		else if (attribute == "rotate" && flag == 1) {
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			int axis = 0;
			float value = 0;

			axis = atoi(parameters.at(0).c_str());
			value = atof(parameters.at(1).c_str());

			if (axis == 0)  // x axis rotation
			{
				rotateX += value;
			}
			else if (axis == 1)
			{
				rotateY += value;
			}
			else if (axis == 2)
			{
				rotateZ += value;
			}
		}
	}

	AnimatedGameObject* obj = new AnimatedGameObject();
	obj->SetCustomMesh(mesh);
	obj->_anim_param = param;
	obj->RotateX(rotateX);
	obj->RotateY(rotateY);
	obj->RotateZ(rotateZ);
	obj->SetAlpha(alpha);
	
	objectList.push_back(obj);
	
	mesh->Initialize(path.c_str());
	AnimatedSprite* anim = dynamic_cast<AnimatedSprite*>(mesh);
	if(frame_width != 0)
		anim->Set(frame_count, fps, loop, frame_width);

	return 0;
}

int GetFBXParameter(ifstream& file)
{
	string data;
	AssimpMesh* mesh = new AssimpMesh();
	glm::vec3 position = glm::vec3(0, 0, 0);
	float alpha = 1.0f;
	AnimatePerFrameParam param;
	float rotateX = 0, rotateY = 0, rotateZ = 0;

	while (getline(file, data))
	{
		cout << data << endl;

		if (data.size() <= 0) continue;
		if (data.c_str()[0] == '#') continue;
		data = RemoveWhitespace(data);

		Tokens tokens;
		Tokenize(data, tokens, "=");
		std::string attribute = tokens.at(0);

		if (attribute == "end") break;
		else if (attribute == "filepath")
		{
			mesh->Load(tokens.at(1));
		}
		else if (attribute == "add_animation")
		{
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			int index, start, end;
			bool loop;

			index = atoi(parameters.at(0).c_str());
			start = atoi(parameters.at(1).c_str());
			end = atoi(parameters.at(2).c_str());
			loop = (parameters.at(3) == "true") ? true : false;
			mesh->AddAnimationByFrame(index, start, end, loop);
		}
		else if (attribute == "set_animation")
		{
			int index = atoi(tokens.at(1).c_str());
			mesh->SetAnimation(index);
		}
		else if (attribute == "position")
		{
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			float x, y, z;

			x = atof(parameters.at(0).c_str());
			y = atof(parameters.at(1).c_str());
			z = atof(parameters.at(2).c_str());

			position = glm::vec3(x, y, z);
		}
		else if (attribute == "update_start")
		{
			GetUpdateParameter(file, &param);
		}
		else if (attribute == "alpha") {
			alpha = atof(tokens.at(1).c_str());
		}
		else if (attribute == "rotate") {
			Tokens parameters;
			Tokenize(tokens.at(1), parameters, ",");
			int axis = 0;
			float value = 0;

			axis = atoi(parameters.at(0).c_str());
			value = atof(parameters.at(1).c_str());

			if (axis == 0)  // x axis rotation
			{
				rotateX += value;
			}
			else if (axis == 1)
			{
				rotateY += value;
			}
			else if (axis == 2)
			{
				rotateZ += value;
			}
		}

	}

	AnimatedGameObject* obj = new AnimatedGameObject();
	obj->SetMesh(mesh);
	obj->SetPosition(position);
	obj->SetAlpha(alpha);
	obj->RotateX(rotateX);
	obj->RotateY(rotateY);
	obj->RotateZ(rotateZ);
	obj->_anim_param = param;
	objectList.push_back(obj);
	
	return 0;
}

void CleanupObjects()
{
	for (int i = 0; i < objectList.size(); i++)
	{
		SAFE_DELETE(objectList.at(i));
	}
	objectList.clear();

}

// Desktop EDItor
bool ReadConfiguration(const char* szConfigFile)
{
	CleanupObjects();
	ifstream file(szConfigFile); //file just has some sentences
	if (!file) {
		cout << "unable to open file";
		return false;
	}
	string sent;
	while (getline(file, sent))
	{
		cout << sent << endl;
		
		if (sent.size() <= 0) continue;
		if (sent.c_str()[0] == '#') continue;

		Tokens tokens;
		Tokenize(sent, tokens, " ");

		if (tokens.at(0) == "add")
		{
			if (tokens.at(1) == "particle")
			{
				GetParticleParameter(file);
			}
			else if (tokens.at(1) == "billboard" )
			{
				GetCustomMeshParameters(file, 0);
			}
			else if (tokens.at(1) == "ground_rectangle")
			{
				GetCustomMeshParameters(file, 1);
			}
			else if (tokens.at(1) == "fbx")
			{
				GetFBXParameter(file);
			}
		}
	}
}

int main(int argc, char* argv[])
{

	if (argc <= 1)
	{
		printf("\n\r Usage: ");
		printf("\n\r Ramo-vfx-editor.exe [effect file].txt");
		return 0;
	}

	// Get Effect file configuration
	strFile = std::string(argv[1]);

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

	camera.LookAt(
		glm::vec3(0, 5, 5),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 0));

	InitializeComponentSystem();

	ReadConfiguration(strFile.c_str());

	grid_shader.Init();
	grid_shader.Load("shaders/vshader_world", GL_VERTEX_SHADER);
	grid_shader.Load("shaders/fshader_world", GL_FRAGMENT_SHADER);
	grid_shader.Build();
	GenerateGrid(50,50);

	glEnable(GL_DEPTH_TEST);
	float lasttime = 0;
	float angle = 0;

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE); //enable this
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	while (!glfwWindowShouldClose(window))
	{
		
		float current_time = glfwGetTime();
		float elapsed = (current_time - lasttime);
		lasttime = current_time;

		camera.Update();

		for(int i = 0;i < objectList.size(); i++)
			UpdateObject(elapsed, objectList.at(i));

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//...
		// Draw the GRID
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

		for (int i = 0; i < objectList.size(); i++)
			DrawObject(objectList.at(i));

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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
		ReadConfiguration(strFile.c_str());
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
