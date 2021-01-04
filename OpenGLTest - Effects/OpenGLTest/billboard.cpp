#include "billboard.h"

Billboard::Billboard() 
	: VBO(0)
	, VAO(0)
	, _shader(0)
	, _alpha(0.0f)
	, _angle(0)
{
}


Billboard::~Billboard()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Billboard::Load()
{
	float vertices[] = {
		// 3D Coordinates	texture coordinates, 
		// in windows, image is upside down, therefore invert texture coordinates
		1.0f,  1.0f, 0.0f,		1.0f, 0.0f,		// top right
		1.0f, -1.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-1.0f,  1.0f, 0.0f,		0.0f, 0.0f,		// top left 
												// second triangle
		1.0f, -1.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-1.0f, -1.0f, 0.0f,		0.0f, 1.0f,		// bottom left
		-1.0f,  1.0f, 0.0f,		0.0f, 0.0f		// top left
	};

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Billboard::SetTexture(const char* szFname)
{
	_texture.LoadFromAssetDirect(szFname);
}

//temporary
int dir = 0;
void Billboard::Update(float elapsed)
{
	float alpha_trans = 1;
	
	if (dir == 0) {
		_alpha += (alpha_trans * elapsed);
		if (_alpha > 1.0f) {
			dir = 1;
		}
	}
	else if (dir == 1) {
		_alpha -= (alpha_trans * elapsed);
		if (_alpha < 0.0f) {
			dir = 0;
		}
	}

	_angle += (20 * elapsed);
}

void Billboard::Render()
{
	if (_shader == NULL) return;

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(VAO);
	_texture.Bind();

	int alphaid = _shader->GetUniformLocation("alpha");
	_shader->SetUniformFloat(alphaid, _alpha);

	float angle = 0;   //360/10 = 36
	for (int i = 0; i < 10; i++) {
		
		glm::mat4 rotateMat = glm::mat4(1.0f);
		rotateMat = glm::rotate(rotateMat, glm::radians(angle), glm::vec3(0, 1, 0));
		rotateMat = glm::rotate(rotateMat, glm::radians(_angle), glm::vec3(0, 1, 0));

		int world = _shader->GetUniformLocation("model");
		_shader->SetUniformMatrix(world, rotateMat);

		glDrawArrays(GL_TRIANGLES, 0, 6);	
		angle += 36.0f;
	}
	
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBindVertexArray(0);
}