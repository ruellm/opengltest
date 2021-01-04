// fog_fx.h
// Fog implemenentation
// Created: Feb 20, 2019
// Author: ruellm@yahoo.com
//Reference
//https://www.youtube.com/watch?v=qslBNLeSPUc
//
#pragma once

#include "graphics/glshaderfx.h"
#include "common.h"

class FogFX : public ramo::graphics::CommonShaderBase
{
public:
	FogFX();
	~FogFX();

	virtual void Load(GLuint program);
	virtual void Use(GLuint program);

	void SetEnable(bool flag);
	void SetColor(float r, float g, float b);
	void SetDensity(float density);
	void SetGradient(float gradient);

private:
	GLuint VS_enableID;
	GLuint FS_enableID;

	GLuint sky_colorID;
	GLuint densityID;
	GLuint gradientID;

	bool   enable;
	glm::vec3 sky_color;

	float density;
	float gradient;
};