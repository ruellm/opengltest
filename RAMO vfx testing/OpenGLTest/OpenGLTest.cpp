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

	camera.LookAt(
		glm::vec3(0, 5, 5),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 0));

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

#if 0 // save this as stretched particle 1 -- used for summon and any other
	ParticleSystem particle;
	particle.LoadTexture("textures/summon_generic/Particle.png");


	float r = 40 / (float)255.0f;
	float g = 10 / (float)255.0f;
	float b = 60 / (float)255.0f;

	particle._startSize = 2;
	particle._maxParticles = 100;
	particle._rateOverTime = 15;
	particle._lifeTime = 5;

	particle._startColor = glm::vec4(r, g, b, 50 / (float)255);
	particle._startSpeed = 10;

	particle._blendMode = PARTICLE_BLENDMODE_ADDITIVE;

	particle.AddColorInterpolator(0.0f, glm::vec4(r, g, b, 0));
	particle.AddColorInterpolator(0.25, glm::vec4(r, g, b, 1));
	particle.AddColorInterpolator(0.5, glm::vec4(r, g, b, 0));
	particle.AddColorInterpolator(0.75, glm::vec4(r, g, b, 1));
	particle.AddColorInterpolator(1, glm::vec4(r, g, b, 0));

	particle._loop = true;
	particle._duration = 0.5;

	particle._scaleH = 5;
	particle._scaleW = 0.2;
	particle._emitter = new ConeEmitter();
	particle._emitter->velocity = glm::vec3(0, 1, 0);
	((ConeEmitter*)particle._emitter)->_radius = 1.0f;
	((ConeEmitter*)particle._emitter)->_radiusThickness = 1.0f;

	GameObject object;
	object.SetParticle(&particle);
	object.SetPosition(glm::vec3(0, 0, 0));
	//object.SetAlpha(0.1);
	objectList.push_back(&object);

	particle.Start();
#endif

#if 0 // HEAL
	ParticleSystem particle;
	particle.LoadTexture("textures/heal/Heal_Glow.png");

	float r = 40 / (float)255.0f;
	float g = 10 / (float)255.0f;
	float b = 60 / (float)255.0f;

	particle._startSize = 0.5;
	particle._maxParticles = 200;
	particle._rateOverTime = 15;
	particle._lifeTime = 5;

	particle._startColor = glm::vec4(r, g, b, 50 / (float)255);
	particle._startSpeed = 10;

	particle._blendMode = PARTICLE_BLENDMODE_ADDITIVE;

	particle.AddColorInterpolator(0.0f, glm::vec4(r, g, b, 0));
	particle.AddColorInterpolator(0.25, glm::vec4(r, g, b, 1));
	particle.AddColorInterpolator(0.5, glm::vec4(r, g, b, 0));
	particle.AddColorInterpolator(0.75, glm::vec4(r, g, b, 1));
	particle.AddColorInterpolator(1, glm::vec4(r, g, b, 0));

	particle._loop = true;
	particle._duration = 0.5;

	particle._scaleH = 3;
	//particle._scaleW = 0.2;
	particle._emitter = new ConeEmitter();
	particle._emitter->velocity = glm::vec3(0, 1, 0);
	((ConeEmitter*)particle._emitter)->_radius = 1.0f;
	((ConeEmitter*)particle._emitter)->_radiusThickness = 1.0f;

	GameObject object;
	object.SetParticle(&particle);
	object.SetPosition(glm::vec3(0, 0, 0));
	//object.SetAlpha(0.1);
	objectList.push_back(&object);

	particle.Start();
#endif

#if 0 // test for unity animation curve - confirmed working, we need to figure out rate over seconds
	ParticleSystem particle;
	particle.LoadTexture("textures/manacrystal/Energy_Ring.png");
	//particle._renderAlign = PARTICLE_RENDER_ALIGN_WORLDRECT;
	//particle._renderAlignRotation.x = glm::radians(90.0f);

	float r = 1;
	float g = 0;
	float b = 0;

	particle._rateOverTime = 8;
	
	particle._lifeTime = 0.5;
	particle.SetStartingAge(0, 0);
	particle.SetStartSize(1.5, 1.5);
	particle.SetStartRotationRad(glm::radians(0.0f), glm::radians(360.0));


	particle.AddColorInterpolator(0.0f, glm::vec4(r, g, b, 0));
	particle.AddColorInterpolator(0.443, glm::vec4(r, g, b, 1));
	particle.AddColorInterpolator(1, glm::vec4(r, g, b, 0));

	particle._startColor = glm::vec4(r, g, b, 1);
	
	particle._blendMode = PARTICLE_BLENDMODE_ADDITIVE;
	particle._flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME_UNITY_CURVE;

	particle._loop = true;
	particle._duration = 0.5;
	//start size
	particle._scaleW = 4;
	particle._scaleH = 4;

	particle._emitter = new CubeEmitter();
	particle._emitter->velocity = glm::vec3(0, 0, 0);
	((CubeEmitter*)particle._emitter)->_thicknessX = 0.0f;
	((CubeEmitter*)particle._emitter)->_thicknessY = 0.0f;
	((CubeEmitter*)particle._emitter)->_thicknessZ = 0.0f;
	
	{
		UnityKeyFrame frame0;
		frame0.time = 0;
		frame0.value = 1.2 * particle._startSize;
		frame0.inSlope = 0;
		frame0.outSlope = 0;

		UnityKeyFrame frame1;
		frame1.time = 1.0f;
		frame1.value = 0;
		frame1.inSlope = 0;
		frame1.outSlope = 0;

		particle._sizeInterpolatorUnityCurve.SetSampleRate(1.0f);	// simulation speed in unity's particle
		particle._sizeInterpolatorUnityCurve.AddKeyFrame(frame0, false);
		particle._sizeInterpolatorUnityCurve.AddKeyFrame(frame1, false);
	}

	GameObject object;
	object.SetParticle(&particle);
	object.SetPosition(glm::vec3(10, 0, 0));
	//object.SetPosition(glm::vec3(57.4884415, 12.7198315, -0.312446117));
	object.SetScale(2, 2, 2);
	//object.SetAlpha(0.1);
	objectList.push_back(&object);

	particle.Start();

#endif

#if 0 // test for unity animation curve - confirmed working, we need to figure out rate over seconds
	ParticleSystem glow;
	//glow.LoadTexture("textures/manacrystal/Glow.png");
	//glow.LoadTexture("textures/S5.png");
	glow.LoadTexture("textures/common_vfx/Ground_Dust.png");

	float r = 1;
	float g = 0;
	float b = 0;

	glow._rateOverTime = 1;
	glow._lifeTime = 2;
	glow.SetStartingAge(0, 0);
	glow._startSize = 1;
	//glow.SetStartSize(1, 1);
//	glow.SetStartRotationRad(glm::radians(0.0f), glm::radians(360.0));

	glow._startColor = glm::vec4(r, g, b, 1);

	glow._blendMode = PARTICLE_BLENDMODE_ADDITIVE;
	glow._flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME_UNITY_CURVE;

	glow._loop = false;
	glow._duration = 1;

	glow._emitter = new CubeEmitter();
	glow._emitter->velocity = glm::vec3(0, 0, 0);
	((CubeEmitter*)glow._emitter)->_thicknessX = 0.0f;
	((CubeEmitter*)glow._emitter)->_thicknessY = 0.0f;
	((CubeEmitter*)glow._emitter)->_thicknessZ = 0.0f;
	
#if 1
	{
		UnityKeyFrame frame0;
		frame0.time = 0;
		frame0.value = 0.426 * glow._startSize;
		frame0.inSlope = 0;
		frame0.outSlope = 0;

		UnityKeyFrame frame1;
		frame1.time = 0.223;
		frame1.value = 0.951 * glow._startSize;
		frame1.inSlope = 0;
		frame1.outSlope = 0;

		UnityKeyFrame frame2;
		frame2.time = 1;
		frame2.value = 0 * glow._startSize;
		frame2.inSlope = 0;
		frame2.outSlope = 0;

		glow._sizeInterpolatorUnityCurve.SetSampleRate(1.0f);	// simulation speed in unity's particle
		glow._sizeInterpolatorUnityCurve.AddKeyFrame(frame0, false);
		glow._sizeInterpolatorUnityCurve.AddKeyFrame(frame1, false);
		glow._sizeInterpolatorUnityCurve.AddKeyFrame(frame2, false);
	}
#endif
	GameObject glowObj;
	glowObj.SetParticle(&glow);
	glowObj.SetPosition(glm::vec3(0, 0, 0));
	//glowObj.SetScale(2, 2, 2);
	//object.SetAlpha(0.1);
	objectList.push_back(&glowObj);

	glow.Start();

#endif


#if 0 // spray test
	ParticleSystem element;
	element.LoadTexture("textures/manacrystal/Glow.png");

//	float r = 1;
//	float g = 1;
//	float b = 1;

	element._rateOverTime = 50;
	element._lifeTime = 0.25;
	element.SetStartingAge(0, 0);
	element.SetStartSize(0.5, 0.5);
	element._startColor = glm::vec4(r, g, b, 1);

	element.AddColorInterpolator(0.0f, glm::vec4(r, g, b, 0));
	element.AddColorInterpolator(0.04, glm::vec4(r, g, b, 1));
	element.AddColorInterpolator(0.91, glm::vec4(r, g, b, 216/(float)255));
	element.AddColorInterpolator(1, glm::vec4(r, g, b, 0));

	element._blendMode = PARTICLE_BLENDMODE_ADDITIVE;
	element._flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME_UNITY_CURVE;

	{
		UnityKeyFrame frame0;
		frame0.time = 0;
		frame0.value = 0 * element._startSize;
		frame0.inSlope = 0;
		frame0.outSlope = 0;

		UnityKeyFrame frame1;
		frame1.time = 0.5;
		frame1.value = 1 * element._startSize;
		frame1.inSlope = 0;
		frame1.outSlope = 0;

		UnityKeyFrame frame2;
		frame2.time = 1;
		frame2.value = 0.5 * element._startSize;
		frame2.inSlope = 0;
		frame2.outSlope = 0;

		element._sizeInterpolatorUnityCurve.SetSampleRate(1.0f);	// simulation speed in unity's particle
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame0, false);
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame1, false);
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame2, false);
	}

	element._loop = true;
	element._duration = 0.5;

#if 0
	glow._emitter = new SphereEmitter();
	glow._emitter->velocity = glm::vec3(0, 0, 0);
	((SphereEmitter*)glow._emitter)->MinInclination = 1;
	((SphereEmitter*)glow._emitter)->MaxInclination = 1;
	((SphereEmitter*)glow._emitter)->MinAzimuth = 1;
	((SphereEmitter*)glow._emitter)->MaxAzimuth = 1;
	((SphereEmitter*)glow._emitter)->MinimumRadius = 0.5;
	((SphereEmitter*)glow._emitter)->MaximumRadius = 1;
#endif

	element._emitter = new SpreadEmitter();
	element._emitter->velocity = glm::vec3(0, 0, 0);
	((SpreadEmitter*)element._emitter)->_thicknessX = 5;
	((SpreadEmitter*)element._emitter)->_thicknessY = 5;
	((SpreadEmitter*)element._emitter)->_thicknessZ = 5;
	
	GameObject elementObj;
	elementObj.SetParticle(&element);
	elementObj.SetPosition(glm::vec3(0, 0, 0));
	elementObj.SetScale(2, 2, 2);
	//object.SetAlpha(0.1);
	objectList.push_back(&elementObj);

	element.Start();

#endif

#if 0 // PARTICLE_RENDER_ALIGN_WORLDRECT testing
	ParticleSystem particle;
	particle.LoadTexture("textures/manacrystal/Energy_Ring.png");
	particle._renderAlign = PARTICLE_RENDER_ALIGN_WORLDRECT;

	float r = 255 / (float)255.0f;
	float g = 255 / (float)255.0f;
	float b = 255 / (float)255.0f;

	//particle._startSize = 2;
	particle.SetStartSize(2, 2);
	particle._rateOverTime = 1;
	particle._lifeTime = 1;

	particle._startColor = glm::vec4(r, g, b, 1.0f);
	particle._startSpeed = 10;

	particle._blendMode = PARTICLE_BLENDMODE_ADDITIVE;

	particle._loop = true;
	particle._duration = 0.5;

	particle._scaleH = 1;
	particle._scaleW = 1;
	particle._renderAlignRotation.x = glm::radians(90.0f);

	particle._emitter = new CubeEmitter();
	particle._emitter->velocity = glm::vec3(0, 0, 0);
	((CubeEmitter*)particle._emitter)->_thicknessX = 0.0f;
	((CubeEmitter*)particle._emitter)->_thicknessY = 0.0f;
	((CubeEmitter*)particle._emitter)->_thicknessZ = 0.0f;

	GameObject object;
	object.SetParticle(&particle);
	object.SetPosition(glm::vec3(10, 0, 0));
	object.SetAlpha(1);
	objectList.push_back(&object);

	particle.Start();
#endif

#if 1 // sphere test positive
	ParticleSystem element;
	element.LoadTexture("textures/common_vfx/Skull.png");

	float r = 153 / (float)255.0f;
	float g = 19 / (float)255.0f;
	float b = 30/(float)255.0f;

	element._rateOverTime = 100;
	element._lifeTime = 0.3;
	element.SetStartingAge(0, 0);
	//element.SetStartSize(0.5, 0.5);
	element._startColor = glm::vec4(r, g, b, 1);
	element._startSpeed = 5.0f;
	element._startSize = 1;

	/*element.AddColorInterpolator(0.0f, glm::vec4(r, g, b, 0));
	element.AddColorInterpolator(0.04, glm::vec4(r, g, b, 1));
	element.AddColorInterpolator(0.91, glm::vec4(r, g, b, 216 / (float)255));
	element.AddColorInterpolator(1, glm::vec4(r, g, b, 0));
	*/
	element._blendMode = PARTICLE_BLENDMODE_ADDITIVE;
	element._flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME_UNITY_CURVE;

	{
		UnityKeyFrame frame0;
		frame0.time = 0;
		frame0.value = 0 * element._startSize;
		frame0.inSlope = 0;
		frame0.outSlope = 0;

		UnityKeyFrame frame1;
		frame1.time = 0.5;
		frame1.value = 1 * element._startSize;
		frame1.inSlope = 0;
		frame1.outSlope = 0;

		UnityKeyFrame frame2;
		frame2.time = 1;
		frame2.value = 0.5 * element._startSize;
		frame2.inSlope = 0;
		frame2.outSlope = 0;

		element._sizeInterpolatorUnityCurve.SetSampleRate(1.0f);	// simulation speed in unity's particle
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame0, false);
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame1, false);
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame2, false);
	}

	element._loop = false;
	element._duration = 0.5;

#if 1
	element._emitter = new SphereEmitter();
	element._emitter->velocity = glm::vec3(0, 0, 0);
	((SphereEmitter*)element._emitter)->MinInclination = 0;
	((SphereEmitter*)element._emitter)->MaxInclination = 360;
	((SphereEmitter*)element._emitter)->MinAzimuth = 0;
	((SphereEmitter*)element._emitter)->MaxAzimuth = 360;
	((SphereEmitter*)element._emitter)->MinimumRadius = 0;
	((SphereEmitter*)element._emitter)->MaximumRadius = 0.3; //0.3 if explosion spread, 3 if from outside to inside
#endif

	GameObject elementObj;
	elementObj.SetParticle(&element);
	elementObj.SetPosition(glm::vec3(0, 0, 0));
	elementObj.SetScale(2, 2, 2);
	objectList.push_back(&elementObj);

	element.Start();

#endif

#if 0 // sphere test negative
	ParticleSystem element;
	element.LoadTexture("textures/manacrystal/Glow.png");

	float r = 1;
	float g = 1;
	float b = 1;

	element._loop = false;
	element._duration = 5;

	element._rateOverTime = 35;
	element._lifeTime = 0.35;
	element.SetStartingAge(0, 0);
	element.SetStartSize(0.5, 0.5);
	element._startColor = glm::vec4(r, g, b, 1);
	element._startSpeed = -5.0f;

	element.AddColorInterpolator(0.0f, glm::vec4(r, g, b, 0));
	element.AddColorInterpolator(0.04, glm::vec4(r, g, b, 1));
	element.AddColorInterpolator(0.91, glm::vec4(r, g, b, 216 / (float)255));
	element.AddColorInterpolator(1, glm::vec4(r, g, b, 0));

	element._blendMode = PARTICLE_BLENDMODE_ADDITIVE;
	element._flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME_UNITY_CURVE;

	{
		UnityKeyFrame frame0;
		frame0.time = 0;
		frame0.value = 0 * element._startSize;
		frame0.inSlope = 0;
		frame0.outSlope = 0;

		UnityKeyFrame frame1;
		frame1.time = 0.5;
		frame1.value = 1 * element._startSize;
		frame1.inSlope = 0;
		frame1.outSlope = 0;

		UnityKeyFrame frame2;
		frame2.time = 1;
		frame2.value = 0.5 * element._startSize;
		frame2.inSlope = 0;
		frame2.outSlope = 0;

		element._sizeInterpolatorUnityCurve.SetSampleRate(1.0f);	// simulation speed in unity's particle
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame0, false);
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame1, false);
		element._sizeInterpolatorUnityCurve.AddKeyFrame(frame2, false);
	}

#if 1
	element._emitter = new SphereEmitter();
	element._emitter->velocity = glm::vec3(0, 0, 0);
	((SphereEmitter*)element._emitter)->MinInclination = 0;
	((SphereEmitter*)element._emitter)->MaxInclination = 360;
	((SphereEmitter*)element._emitter)->MinAzimuth = 0;
	((SphereEmitter*)element._emitter)->MaxAzimuth = 360;
	((SphereEmitter*)element._emitter)->MinimumRadius = 2;
	((SphereEmitter*)element._emitter)->MaximumRadius = 2; //0.3 if explosion spread, 3 if from outside to inside
#endif

	GameObject elementObj;
	elementObj.SetParticle(&element);
	elementObj.SetPosition(glm::vec3(0, 0, 0));
	elementObj.SetScale(2, 2, 2);
	objectList.push_back(&elementObj);

	element.Start();

#endif

#if 0
	ZoomRectVFX zoomfx;
	zoomfx.Initialize(0);
	objectList.push_back(&zoomfx);
#endif

#if 0
	HealingVFX heal;
	heal.Initialize();
	objectList.push_back(&heal);
#endif

#if 0 // zooming circle
	ParticleSystem particle;
	particle.LoadTexture("textures/common_vfx/0b.png");
	particle._renderAlign = PARTICLE_RENDER_ALIGN_WORLDRECT;

	float r = 255 / (float)255.0f;
	float g = 255 / (float)255.0f;
	float b = 255 / (float)255.0f;

	particle.SetStartSize(2, 2);
	particle._rateOverTime = 3;
	particle._lifeTime = 1;

	particle._startColor = glm::vec4(r, g, b, 1.0f);
	particle._startSpeed = 10;

	particle.AddColorInterpolator(0.0f, glm::vec4(r, g, b, 0));
	particle.AddColorInterpolator(0.70, glm::vec4(r, g, b, 247/(float)255));
	particle.AddColorInterpolator(1, glm::vec4(r, g, b, 0));

	particle._blendMode = PARTICLE_BLENDMODE_ADDITIVE;

	particle._flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME_UNITY_CURVE;
	{
		UnityKeyFrame frame0;
		frame0.time = 0;
		frame0.value = 0.313 * particle._startSize;
		frame0.inSlope = 0;
		frame0.outSlope = 0;

		UnityKeyFrame frame1;
		frame1.time = 0.339;
		frame1.value = 0.821 * particle._startSize;
		frame1.inSlope = 0;
		frame1.outSlope = 0;

		UnityKeyFrame frame2;
		frame2.time = 1;
		frame2.value = 1 * particle._startSize;
		frame2.inSlope = 0;
		frame2.outSlope = 0;

		particle._sizeInterpolatorUnityCurve.SetSampleRate(1.0f);	// simulation speed in unity's particle
		particle._sizeInterpolatorUnityCurve.AddKeyFrame(frame0, false);
		particle._sizeInterpolatorUnityCurve.AddKeyFrame(frame1, false);
		particle._sizeInterpolatorUnityCurve.AddKeyFrame(frame2, false);
	}

	particle._loop = true;
	particle._duration = 0.5;

	particle._scaleH = 1;
	particle._scaleW = 1;
	particle._renderAlignRotation.x = glm::radians(90.0f);

	particle._emitter = new CubeEmitter();
	particle._emitter->velocity = glm::vec3(0, 0, 0);
	((CubeEmitter*)particle._emitter)->_thicknessX = 0.0f;
	((CubeEmitter*)particle._emitter)->_thicknessY = 0.0f;
	((CubeEmitter*)particle._emitter)->_thicknessZ = 0.0f;

	GameObject object;
	object.SetParticle(&particle);
	object.SetPosition(glm::vec3(0, 0, 0));
	object.SetAlpha(1);
	objectList.push_back(&object);

	particle.Start();
#endif

#if 0
	BuffDebuffVFX buff;
	buff.Initialize(BUFF_DEBUFF_VFX_BUFF, 1);
	buff.SetPosition(glm::vec3(10, 0, 0));
	objectList.push_back(&buff);

	BuffDebuffVFX buff2;
	buff2.Initialize(BUFF_DEBUFF_VFX_DEBUFF, 1);
	buff2.SetPosition(glm::vec3(0, 0, 0));
	objectList.push_back(&buff2);
#endif

#if 0
	MagicCastVFX magic;
	magic.Initialize();
	magic.SetPosition(glm::vec3(0, 0, 0));
	objectList.push_back(&magic);
#endif


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
