// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "assimp.lib")

#include "common.h"
#include <iostream>
#include <algorithm>    // std::max
#include "GLShaderFx.h"
#include "3rd_party/stb_image.h"
#include "obj_model.h"
//#include "ModelShaderFx.h"
//#include "lightning_fx.h"
#include "scene/Camera.h"

#include "assimp_mesh.h"
#include "texture2D.h"

#include "graphics/FBO.h"

#include "hex_tile.h"
#include "utility.h"

// settings
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void BuildRay(int x, int y, glm::vec3& start, glm::vec3& dir);
bool MousePick(const glm::vec3& rayPos,
	const glm::vec3& rayDir, glm::vec3* hitPos, glm::vec3 min, glm::vec3 max, bool oponent=false);

GLenum error;

// for terrain object
GLShaderFx shader;

// hexagon definitions
Camera camera;

// Projection matrix 4:3 ratio
glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f),
	4.0f / 3.0f, 1.0f, 1000.0f);

// Camera matrix
glm::mat4 viewMatrix;

int _playerIndex = 0;

// hex tiles 
GLShaderFx hexShader;

static std::vector<HexTile*> hex_tiles;
static std::vector<HexTile*> opponent_tiles;

Texture2D hexTexture;


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

const unsigned int SCR_WIDTH = 960;
const unsigned int SCR_HEIGHT = 540;
float object_x, object_y, object_z;
float nextX, nextZ;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR	, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR	, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE			, GLFW_OPENGL_ANY_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RAMO Engine", NULL, NULL);
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

	shader.Init();
	shader.LoadVertexShaderFromFile("./shaders/model_vshader.txt");
	shader.LoadFragmentShaderFromFile("./shaders/model_fshader.txt");
	shader.Build();

	hexShader.Init();
	hexShader.LoadVertexShaderFromFile("./shaders/model_vshader.txt");
	hexShader.LoadFragmentShaderFromFile("./shaders/model_fshader_glow.txt");
	hexShader.Build();

	GLShaderFx brightFilter;
	brightFilter.Init();
	brightFilter.LoadVertexShaderFromFile("./shaders/model_vshader.txt");
	brightFilter.LoadFragmentShaderFromFile("./shaders/brightness_filter_fshader.txt");
	brightFilter.Build();

	// render loop
	// -----------
	int textureID = 0;
	float lasttime = glfwGetTime();

	//camera.LookAt(glm::vec3(0, 48, 65), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));

	camera.LookAt(glm::vec3(0, 5, 5), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));

	AssimpMesh tree;
	tree.Load("./models/ravine_level_tree.fbx");

	AssimpMesh crystal;
	crystal.Load("./models/crystal.fbx");

	float start_time = glfwGetTime();

	AssimpMesh hex;
	hex.Load("./models/hex_no_design.fbx");

	AssimpMesh hexGlow;
	hexGlow.Load("./models/hex_no_design_GLOW.fbx");

	AssimpMesh beast1;
	beast1.Load("./models/Beast1/Beast1.fbx");

	AssimpMesh fireSentinel;
	fireSentinel.Load("./models/C3/C3.fbx");
	
	AssimpMesh A1;
	A1.Load("./models/A1/A1-3D.fbx");

	AssimpMesh A2;
	A2.Load("./models/A2/A2-3D.fbx");

	const float hexWidth = 16.819;
	const float hexHeight = 14.576;
	const float hexDistanceX = 12.85;
	const float diffWidth = hexWidth - hexDistanceX;
	const float hexDistanceZ = -7.85;

	while (!glfwWindowShouldClose(window))
	{
		float current_time = glfwGetTime();
		float elapsed = (current_time - lasttime);
		lasttime = current_time;
		
		// input
		// -----
		processInput(window);

		camera.Update();
		viewMatrix = camera.GetViewMatrix();

		for (int i = 0; i < hex_tiles.size(); i++) {
			hex_tiles.at(i)->Update(elapsed);
			opponent_tiles.at(i)->Update(elapsed);
		}

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 model = glm::mat4(1.0f);		//identity
		glEnable(GL_DEPTH_TEST);

		shader.Use();
		int gWVP = shader.GetUniformLocation("gVP");
		int gWorld = shader.GetUniformLocation("gWorld");
		int alphaID = shader.GetUniformLocation("alpha");

		shader.SetUniformMatrix(gWVP, projectionMatrix * viewMatrix);
		shader.SetUniformMatrix(gWorld, model);
		shader.SetUniformFloat(alphaID, 1.0f);

		textureID = shader.GetUniformLocation("gSampler");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture

			// render tree and crystal together

		tree.Render();
		crystal.Render();
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		hexShader.Use();
		gWVP = hexShader.GetUniformLocation("gVP");
		gWorld = hexShader.GetUniformLocation("gWorld");
		alphaID = hexShader.GetUniformLocation("alpha");

		model = glm::mat4(1.0f);		//identity
		hexShader.SetUniformMatrix(gWVP, projectionMatrix * viewMatrix);
		hexShader.SetUniformMatrix(gWorld, model);
		hexShader.SetUniformFloat(alphaID, 1.0f);

		textureID = hexShader.GetUniformLocation("gSampler");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
		
		float distanceFromOrigin = 30;
		float totalWidth = hexDistanceX * 6;
		float currX = -(totalWidth / 2);
		float currZ = distanceFromOrigin;
		float currXOponent = -currX;

		//temporary
		std::vector<glm::vec3> hexpos;
		std::vector<glm::vec3> hexposOponent;

		for (int t = 0; t < 7; t++)
		{
			if ((t % 2) != 0)
				currZ = distanceFromOrigin + hexDistanceZ;
			else
				currZ = distanceFromOrigin;

			model = glm::mat4(1.0f);		//identity
			model = glm::translate(model, glm::vec3(currX, 0, currZ));
			hexShader.SetUniformMatrix(gWorld, model);
			hexGlow.Render();

			hexpos.push_back(glm::vec3(currX, 0, currZ));

			model = glm::mat4(1.0f);		//identity
			model = glm::translate(model, glm::vec3(currXOponent, 0, -currZ));
			hexShader.SetUniformMatrix(gWorld, model);
			hexGlow.Render();

			hexposOponent.push_back(glm::vec3(currXOponent, 0, -currZ));

			currX += hexDistanceX;
			currXOponent -= hexDistanceX;
		}

		// ADD monster
		for (int i=0;i< hexpos.size();i++)
		{
			shader.Use();
			int gWVP = shader.GetUniformLocation("gVP");
			int gWorld = shader.GetUniformLocation("gWorld");
			int alphaID = shader.GetUniformLocation("alpha");

			glm::mat4 model = glm::mat4(1.0f);		//identity
			shader.SetUniformMatrix(gWVP, projectionMatrix * viewMatrix);
			shader.SetUniformMatrix(gWorld, model);
			shader.SetUniformFloat(alphaID, 1.0f);

			textureID = shader.GetUniformLocation("gSampler");
			glUniform1i(textureID, 0);				// set the textureID to texture unit 0
			glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
		
			model = glm::mat4(1.0f);		//identity
			glm::vec3 position = hexpos.at(i);
			glm::mat4 rotateY = glm::mat4(1.0f);
			float angle = glm::radians(180.0f);
			rotateY  = glm::rotate(rotateY, angle, glm::vec3(0.0f, 1.0f, 0.0f));

			model = glm::translate(model, position);
			hexShader.SetUniformMatrix(gWorld, model * rotateY);

			if (i % 2 == 1)
				beast1.Render();
			else
				fireSentinel.Render();
		}
		
		for (int i = 0; i < hexposOponent.size(); i++)
		{
			shader.Use();
			int gWVP = shader.GetUniformLocation("gVP");
			int gWorld = shader.GetUniformLocation("gWorld");
			int alphaID = shader.GetUniformLocation("alpha");

			glm::mat4 model = glm::mat4(1.0f);		//identity
			shader.SetUniformMatrix(gWVP, projectionMatrix * viewMatrix);
			shader.SetUniformMatrix(gWorld, model);
			shader.SetUniformFloat(alphaID, 1.0f);

			textureID = shader.GetUniformLocation("gSampler");
			glUniform1i(textureID, 0);				// set the textureID to texture unit 0
			glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture

			model = glm::mat4(1.0f);		//identity
			glm::vec3 position = hexposOponent.at(i);

			model = glm::translate(model, position);
			hexShader.SetUniformMatrix(gWorld, model);

			if (i % 2 == 1)
				beast1.Render();
			else
				fireSentinel.Render();
		}

		model = glm::mat4(1.0f);		//identity
		glm::mat4 rotateY = glm::mat4(1.0f);
		float angle = glm::radians(180.0f);
		rotateY = glm::rotate(rotateY, angle, glm::vec3(0.0f, 1.0f, 0.0f));

		model = glm::translate(model, glm::vec3(0, 0, distanceFromOrigin+ hexHeight));
		shader.SetUniformMatrix(gWorld, model * rotateY);
		A1.Render();

		model = glm::mat4(1.0f);		//identity
		model = glm::translate(model, glm::vec3(0, 0, -(distanceFromOrigin+hexHeight)));
		shader.SetUniformMatrix(gWorld, model);
		A2.Render();

		//hexGlow.Render();

		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		
		//DrawHexTiles();
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

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.Walk(-0.10f);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.Walk(0.10f);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.Strafe(-0.05f);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.Strafe(0.05f);
	}
	
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		camera.Fly(0.001f);
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		camera.Fly(-0.01f);
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

bool mbutton_down = false;
double lastx, lasty;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
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
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (mbutton_down) {
		// mouse move
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		double diffx = (xpos - lastx) * 0.5;
		double diffy = (ypos - lasty) * 0.5;

		camera.Yaw(-diffx);
		camera.Pitch(-diffy);

		lastx = xpos;
		lasty = ypos;

	}
}

void BuildRay(int x, int y, glm::vec3& start, glm::vec3& dir)
{
	glm::vec4 viewport = glm::vec4(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// why do we need to invert screen Y coordinates?
	start = glm::unProject(
		glm::vec3(x, SCR_HEIGHT - y, 0.0),
		viewMatrix, projectionMatrix, viewport);

	glm::vec3 end = glm::unProject(
		glm::vec3(x, SCR_HEIGHT - y, 1.0),
		viewMatrix, projectionMatrix, viewport);

	dir = glm::normalize(end - start);
}

static bool RaySlabIntersect(float slabmin, float slabmax, float raystart, float rayend, float& tbenter, float& tbexit)
{
	float raydir = rayend - raystart;

	// ray parallel to the slab
	if (fabs(raydir) < 1.0E-9f)
	{
		// ray parallel to the slab, but ray not inside the slab planes
		if (raystart < slabmin || raystart > slabmax)
		{
			return false;
		}
		// ray parallel to the slab, but ray inside the slab planes
		else
		{
			return true;
		}
	}

	// slab's enter and exit parameters
	float tsenter = (slabmin - raystart) / raydir;
	float tsexit = (slabmax - raystart) / raydir;

	// order the enter / exit values.
	if (tsenter > tsexit)
	{
		//swapf(tsenter, tsexit);
		float temp = tsenter;
		tsenter = tsexit;
		tsexit = temp;
	}

	// make sure the slab interval and the current box intersection interval overlap
	if (tbenter > tsexit || tsenter > tbexit)
	{
		// nope. Ray missed the box.
		return false;
	}
	// yep, the slab and current intersection interval overlap
	else
	{
		// update the intersection interval
		tbenter = std::max(tbenter, tsenter);
		tbexit = std::min(tbexit, tsexit);
		return true;
	}
}/*--------------------------------------------------------------*/

// DO NOT USE THIS! use the original from Phoenix engine
bool MousePick(const glm::vec3& rayPos,
	const glm::vec3& rayDir, glm::vec3* hitPos, glm::vec3 min, glm::vec3 max, bool oponent )
{

//	glm::mat4 worldMat = m_worldMat * m_bbox->GetTransformMatrix();
//	glm::vec4 min = m_worldMat * glm::vec4(m_bbox->min, 1);
//	glm::vec4 max = m_worldMat * glm::vec4(m_bbox->max, 1);
	glm::vec3 rayEnd = rayPos + (rayDir * 100.0f);		   // Multiplied to Far clip plane
														   // initialise to the segment's boundaries. 
	if (oponent) {
		glm::mat4 model = glm::mat4(1.0f);		
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0, 0.0, 1.0));

		//the image is 1184x1024, or (1 x 0.86)
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 0.86f));
		min = model * glm::vec4(-min, 1);
		max = model * glm::vec4(-max, 1);

	}

	float tenter = 0.0f, texit = 1.0f;

	// test X slab
	if (!RaySlabIntersect(min.x, max.x, rayPos.x, rayEnd.x, tenter, texit))
	{
		return false;
	}

	// test Y slab

	if (!RaySlabIntersect(min.y, max.y, rayPos.y, rayEnd.y, tenter, texit))
	{
		return false;
	}

	// test Z slab
	if (!RaySlabIntersect(min.z, max.z, rayPos.z, rayEnd.z, tenter, texit))
	{
		return false;
	}

	// all intersections in the green. Return the first time of intersection, tenter.
	//tinter = tenter;
	return  true;
}