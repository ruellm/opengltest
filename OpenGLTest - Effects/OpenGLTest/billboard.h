#pragma once
#include "common.h"
#include "texture2D.h"

#include "GLShaderFx.h"

class Billboard
{
public:
	Billboard();
	~Billboard();

	void SetTexture(const char* szFname);
	void Load();
	void Update(float elapsed);
	void Render();

	//TODO: temporary? finalize this in actual game
	GLShaderFx* _shader;

private:
	GLuint		VBO;
	GLuint		VAO;
	Texture2D	_texture;
	float		_alpha;
	float		_angle;
};

