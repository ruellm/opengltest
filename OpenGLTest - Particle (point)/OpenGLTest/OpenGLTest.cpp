// OpenGLTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "assimp.lib")

#include "common.h"
#include <iostream>
#include <algorithm>    // std::max
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include "GLShaderFx.h"
#include "stb_image.h"
#include "obj_model.h"
#include "ModelShaderFx.h"
#include "lightning_fx.h"
#include "Camera.h"

#include "assimp_mesh.h"
#include "texture2D.h"

#include "particle_system.h"
#include "trail_particle.h"

// settings
const unsigned int SCR_WIDTH	= 800;
const unsigned int SCR_HEIGHT	= 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void BuildRay(int x, int y, glm::vec3& start, glm::vec3& dir);
bool MousePick(const glm::vec3& rayPos,
	const glm::vec3& rayDir, glm::vec3* hitPos, glm::vec3 min, glm::vec3 max, bool oponent=false);

GLenum error;


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
	4.0f / 3.0f, 1.0f, 100.0f);

// Camera matrix
glm::mat4 viewMatrix; 
float object_x, object_y, object_z;
int main()
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
	shader.LoadVertexShaderFromFile("./shaders/particle_shader.vs");
	shader.LoadFragmentShaderFromFile("./shaders/particle_shader.fs");
	shader.Build();
	srand(time(NULL));

#if 1
	ParticleEmmitter explode;
	explode._position = glm::vec3(0, 0, 0);
	explode._color = glm::vec4(1, 1, 0, 1);
	explode._life_flag = true;
	explode._max_life_time = 3.0f;
	explode._min_direction = glm::vec3(0, 0, 0);
	explode._max_direction = glm::vec3(0, 1, 0);
	explode._emit_radius = glm::vec3(0, 0, 0);

	explode._direction_ramdomized[0] = true;
	explode._direction_x_random.Set(-0.25f, 0.25f);

	explode.Generate(1000);
#endif
#if 0
	TrailParticle trail;
	trail._position = glm::vec3(0, 0, 0);
	trail._color = glm::vec4(1, 1, 0, 1);
	trail._goto_direction = glm::vec3(1, 0, 0);
	trail._max_life_time = 3.0f;
	trail._emit_radius = glm::vec3(0.2, 0.2, 0.2);
	trail._target = glm::vec3(5, 0, 0);

	trail._life_flag = true;
	trail._tail_level = 5;
	trail.Generate(100);
#endif
	// render loop
	// -----------
	int textureID = 0;
	float lasttime = glfwGetTime();

	camera.LookAt(glm::vec3(0, 0, 10), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);

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

		//explode.Update(elapsed);
		explode.Update(elapsed);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();

		glm::mat4 model = glm::mat4(1.0f);		//identity
		int gVP = shader.GetUniformLocation("gVP");
		int gWorld = shader.GetUniformLocation("gWorld");

		glm::mat4 VP = projectionMatrix * viewMatrix;  // <-- this cause a problem sometimes

		shader.SetUniformMatrix(gVP, VP);
		shader.SetUniformMatrix(gWorld, model);
		explode.Render();
		//trail.Render();

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


