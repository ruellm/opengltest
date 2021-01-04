// point_light_fx.h
// definitions for common point lights
// created: February 21, 2019
// ruellm@yahoo.com
// Reference
// 
// Note: Currently disable support for Attenuation

#pragma once
#include "graphics/glshaderfx.h"
#include "common.h"

#define	MAX_POINT_LIGHT_COUNT	4

struct PointLight
{
	glm::vec3 position;
	glm::vec3 color;
	//glm::vec3 attenuation;
};

class PointLightFX 
	: public ramo::graphics::CommonShaderBase
{
public:
	PointLightFX();
	~PointLightFX();

	virtual void Load(GLuint program);
	virtual void Use(GLuint program);

	void Add(PointLight& point);

	std::vector<PointLight> light_list;

private:
	int lightCount;

	GLuint	lightCountVSID;
	GLuint	lightCountFSID;
	GLuint	lightPositionID[MAX_POINT_LIGHT_COUNT];	
	//GLuint	attenuationID[MAX_POINT_LIGHT_COUNT];
	GLuint	lightColorID[MAX_POINT_LIGHT_COUNT];
};