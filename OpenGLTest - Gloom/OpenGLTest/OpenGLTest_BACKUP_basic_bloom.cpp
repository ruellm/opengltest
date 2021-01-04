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
#include "stb_image.h"
#include "obj_model.h"
#include "ModelShaderFx.h"
#include "lightning_fx.h"
#include "Camera.h"

#include "assimp_mesh.h"
#include "texture2D.h"

#include "FBO.h"


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

	/////////////////////////////////////////////////////////////////
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

	unsigned int scrrect_VBO = 0;
	unsigned int  scrrect_VAO = 0;

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
	/////////////////////////////////////////////////////////////////


	// render loop
	// -----------
	int textureID = 0;
	float lasttime = glfwGetTime();

	camera.LookAt(glm::vec3(0, 23, 47), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));

	//GLuint TIF_Test = 0;
	//LoadTexture(&TIF_Test, "./Texture/test2.dds");

	AssimpMesh tree;
	tree.Load("./models/tree.fbx");

	AssimpMesh crystal;
	crystal.Load("./models/crystal.fbx");

	//Texture2D modelTexture;
	//modelTexture.LoadFromAssetDirect("./models/B1-Texture_U1_V1.png");

	float start_time = glfwGetTime();
	glEnable(GL_DEPTH_TEST);

	FBO color1;
	FBO color2;

	color1.Initialize(SCR_WIDTH, SCR_HEIGHT, true);
	color2.Initialize(SCR_WIDTH, SCR_HEIGHT, true);

	GLShaderFx shaderOrtho;
	shaderOrtho.Init();
	shaderOrtho.LoadVertexShaderFromFile("./shaders/basic_vshader_ortho.txt");
	shaderOrtho.LoadFragmentShaderFromFile("./shaders/basic_fshader_ortho.txt");
	shaderOrtho.Build();

	FBO pingPong[2];
	pingPong[0].Initialize(SCR_WIDTH, SCR_HEIGHT);
	pingPong[1].Initialize(SCR_WIDTH, SCR_HEIGHT);

	GLShaderFx blurShader;
	blurShader.Init();
	blurShader.LoadVertexShaderFromFile("./shaders/basic_vshader_ortho.txt");
	blurShader.LoadFragmentShaderFromFile("./shaders/blur_fs.txt");
	blurShader.Build();

	GLShaderFx bloomFinal;
	bloomFinal.Init();
	bloomFinal.LoadVertexShaderFromFile("./shaders/basic_vshader_ortho.txt");
	bloomFinal.LoadFragmentShaderFromFile("./shaders/bloom_final_fs.txt");
	bloomFinal.Build();

	FBO finalScene;
	finalScene.Initialize(SCR_WIDTH, SCR_HEIGHT);

	float alpha = 0;
	int alpha_dir = 0;

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

		if (alpha_dir == 0)
		{
			alpha += 2.0f * elapsed;
			if (alpha >= 1.0f) {
				alpha_dir = 1;
				alpha = 1.0f;
			}
		}
		else if (alpha_dir == 1)
		{
			alpha -= 2.0f * elapsed;
			if (alpha <= 0.0f) {
				alpha_dir = 0;
				alpha = 0.0f;
			}
		}

		// render
		// ------
		color1.Bind();
		
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
		color1.Unbind();

		// Render crystal
		color2.Bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader.SetUniformFloat(alphaID, alpha);
			crystal.Render();
		color2.Unbind();

#if 1
		//...
		//Do Gausian Blur
		//https://learnopengl.com/Advanced-Lighting/Bloom
		// --------------------------------------------------
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 10;
		blurShader.Use();

		for (unsigned int i = 0; i < amount; i++)
		{
			pingPong[horizontal].Bind();

			int horizontalId = blurShader.GetUniformLocation("horizontal");
			blurShader.SetUniformInt(horizontalId, horizontal);

			textureID = blurShader.GetUniformLocation("image");
			glUniform1i(textureID, 0);				// set the textureID to texture unit 0
			glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture

			glBindTexture(GL_TEXTURE_2D, first_iteration ? color2.GetColorTexture() 
				: pingPong[!horizontal].GetColorTexture());	// bind the render target texture	

			// RenderQuad
			glBindVertexArray(scrrect_VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;

			pingPong[horizontal].Unbind();
		}

		// final combine
		bloomFinal.Use();

		int hdr = bloomFinal.GetUniformLocation("scene");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
		glBindTexture(GL_TEXTURE_2D, color1.GetColorTexture());

		int bloomTexture = bloomFinal.GetUniformLocation("bloomBlur");
		glUniform1i(bloomTexture, 1);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE1);			// activate the texture unit 0 first before binding texture
		glBindTexture(GL_TEXTURE_2D, pingPong[!horizontal].GetColorTexture());

		finalScene.Bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindVertexArray(scrrect_VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		finalScene.Unbind();
#endif
		//-----------------------------------------------------------------------------
#if 1
		// Restore frame buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// activate the texture unit 0 first before binding texture
		shaderOrtho.Use();

		textureID = shaderOrtho.GetUniformLocation("texture");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
		glBindTexture(GL_TEXTURE_2D, finalScene.GetColorTexture());	// bind the render target texture

		glBindVertexArray(scrrect_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		error = glGetError();
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

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.Walk(-0.10f);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
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

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		camera.Yaw(0.001f);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		camera.Yaw(-0.001f);
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
		camera.Pitch(0.001f);
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		camera.Pitch(-0.001f);
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		//...
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

		double diffx = xpos - lastx;
		double diffy = ypos - lasty;
		if (diffx > 0) {
			camera.Yaw(0.001f);
		}
		else {
			camera.Yaw(-0.001f);
		}

		if (diffy > 0) {
			camera.Pitch(0.001f);
		}
		else {
			camera.Pitch(-0.001f);
		}

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