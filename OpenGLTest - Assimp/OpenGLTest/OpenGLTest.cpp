// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "assimp.lib")

#include "common.h"
#include <iostream>
#include <algorithm>    // std::max
#include "graphics/GLShaderFx.h"
#include "3rd_party/stb_image.h"
#include "graphics/obj_model.h"
//#innclude "ModelShaderFx.h"
//#include "lightning_fx.h"
#include "scene/camera.h"

#include "graphics/assimp_mesh.h"
#include "graphics/texture2D.h"

using namespace ramo::graphics;

// settings
const unsigned int SCR_WIDTH	= 960;
const unsigned int SCR_HEIGHT	= 540;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void BuildRay(int x, int y, glm::vec3& start, glm::vec3& dir);
bool MousePick(const glm::vec3& rayPos,
	const glm::vec3& rayDir, glm::vec3* hitPos, glm::vec3 min, glm::vec3 max, bool oponent=false);

GLenum error;

// for terrain object
GLuint VBO_terrain;
GLuint VAO_terrain;
GLShaderFx shader;
GLuint textureTerrain;

// hexagon definitions
GLuint VBO_hexagon;
GLuint VAO_hexagon;
GLuint hexagonTexture;

bool show_volt = false;
Camera camera;

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

// Projection matrix 4:3 ratio
glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f),
	4.0f / 3.0f, 1.0f, 1000.0f);

// Camera matrix
glm::mat4 viewMatrix; 
float object_x, object_y, object_z;
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR	, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR	, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE			, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Agila 1.0", NULL, NULL);
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
	shader.LoadVertexShaderFromFile("./shaders/vshader_world.txt");
	shader.LoadFragmentShaderFromFile("./shaders/basic_fshader.txt");
	shader.Build();

	// render loop
	// -----------
	int textureID = 0;
	float lasttime = glfwGetTime();

	camera.LookAt(glm::vec3(0, 5, 10), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));

	GLShaderFx skinshader;
	skinshader.Init();
	skinshader.LoadVertexShaderFromFile("./shaders/skinning.vs");
	skinshader.LoadFragmentShaderFromFile("./shaders/skinning.fs");
	skinshader.Build();
	error = glGetError();
	
	//-----------------------------------------------------------
	//This is temporary skined mesh model will populate the bones
/*
	std::vector<glm::mat4> boneTransforms;

	for (int bt = 0; bt < MAX_BONES; bt++) {
		boneTransforms.push_back(glm::mat4(1.0f));
	}
*/
	//-----------------------------------------------------------

	AssimpMesh mesh;
	//mesh.Load("./models/F_Rune.fbx");
//	mesh.Load("./models/priest/Hellmech_Priest.FBX");
	//mesh.Load("./models/barbarian/barbarian_girl.fbx");
	//mesh.Load("./models/chibi/Chibi_Character_Up.FBX");
//	mesh.Load("./models/bob/boblampclean.md5mesh");
	//mesh.Load("./models/ghoul/micro_ghoul_animated.fbx");
	//mesh.Load("./models/barbarian/barbarian_girl.fbx");
	//mesh.Load("./models/ranger/Orc_ranger_anim.FBX");
	mesh.Load("./models/C3/C3 - 3D.fbx");  //-- loads the texture inside
	//mesh.Load("./models/B3-Battlefield/B3 - battlefield & Crystal.fbx");

	//Texture2D priestTexture;
	//priestTexture.LoadFromFile("./models/F_Rune_texture.png");
	//priestTexture.LoadFromFile("./models/chibi/Chibi_Body_DF.png");
	//priestTexture.LoadFromFile("./models/ghoul/micro_ghoul_col.tga");
	//priestTexture.LoadFromFile("./models/barbarian/barbarian_girl.tga");
	//priestTexture.LoadFromAssetDirect("./models/ranger/Orc_Ranger_Blue.png");
	//priestTexture.LoadFromFile("./models/priest/Hellmech_Priest_Diffuse.jpg");

//	AssimpMesh cube;
	//cube.Load("./models/cube.FBX");

	float start_time = glfwGetTime();
	glEnable(GL_DEPTH_TEST);
	float angle = 0.0f;

	//mesh.AddAnimationByTime(0, 0.0f, 3.0f, true);
	mesh.AddAnimationByFrame(0, 94, 125, true);
	mesh.SetAnimation(0);

	while (!glfwWindowShouldClose(window))
	{
		float current_time = glfwGetTime();
		float elapsed = (current_time - lasttime);
		lasttime = current_time;

		angle += (20 * elapsed);

		// input
		// -----
		processInput(window);

		camera.Update();
		viewMatrix = camera.GetViewMatrix();

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0f);		//identity

		float nowTime = glfwGetTime();
		float runnTime = nowTime - start_time;

		mesh.Update(elapsed);

		//mesh.BoneTransform(runnTime, boneTransforms);
		skinshader.Use();

		// Draw the Assimp mesh
		int gWVP = skinshader.GetUniformLocation("gWVP");
		int gWorld = skinshader.GetUniformLocation("gWorld");

		int gBones[100];
		for (int b = 0; b < mesh.BoneTransforms.size(); b++) {
			char format[MAX_BONES];
			sprintf(format, "gBones[%d]", b);
			gBones[b] = skinshader.GetUniformLocation(format);

			// set bon transform temporary
			glm::mat4 matrix = mesh.BoneTransforms.at(b);
			skinshader.SetUniformMatrix(gBones[b], matrix);
		}

		//model = glm::scale(model, glm::vec3(0.05, 0.05, 0.05));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0f, 0.0f));
		skinshader.SetUniformMatrix(gWVP, projectionMatrix * viewMatrix * model);
		skinshader.SetUniformMatrix(gWorld, model);
		int withBonesID = skinshader.GetUniformLocation("withBones");
		skinshader.SetUniformInt(withBonesID, (mesh.IsAnimated()) ? 1 : 0);
		
		textureID = shader.GetUniformLocation("gSampler");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
				
		//priestTexture.Bind();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		mesh.Render();
		error = glGetError();

		glDisable(GL_BLEND);

#if 0  //testing to get bone matrix
		std::string bones[] = { "Bip001 Head", "Bip001 L Hand", "Bip001 R Hand" };		
		for (int i = 0; i < 3; i++) {
			model = mesh.GetBoneMatrixByName((char*)bones[i].c_str());  //"
			model = glm::scale(model, glm::vec3(15, 15, 15));
			skinshader.SetUniformMatrix(gWVP, projectionMatrix * viewMatrix * model);
			skinshader.SetUniformMatrix(gWorld, model);
			withBonesID = skinshader.GetUniformLocation("withBones");
			skinshader.SetUniformInt(withBonesID, 0);
			cube.Render();
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

		double diffx = (xpos - lastx) * 0.05;
		double diffy = (ypos - lasty) * 0.05;

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