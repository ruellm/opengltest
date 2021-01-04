#include "point_light_fx.h"

PointLightFX::PointLightFX() 
	: lightCount(0), lightCountVSID(0), lightCountFSID(0)
{
	light_list.clear();

	memset(lightPositionID, 0, sizeof(GLuint) * MAX_POINT_LIGHT_COUNT);
	//memset(attenuationID, 0, sizeof(GLuint) * MAX_POINT_LIGHT_COUNT);
	memset(lightColorID, 0, sizeof(GLuint) * MAX_POINT_LIGHT_COUNT);
}

PointLightFX::~PointLightFX()
{
	
}

void PointLightFX::Load(GLuint program)
{
	lightCountVSID = GetUniformLocation("lightCountVS");
	lightCountFSID = GetUniformLocation("lightCountFS");

	char format[20];
	for (int i = 0; i < MAX_POINT_LIGHT_COUNT; i++)
	{
		sprintf(format, "lightPosition[%d]", i);
		lightPositionID[i] = GetUniformLocation(format);

		sprintf(format, "lightColor[%d]", i);
		lightColorID[i] = GetUniformLocation(format);

		//sprintf(format, "attenuation[%d]", i);
		//attenuationID[i] = GetUniformLocation(format);
	}
}

void PointLightFX::Use(GLuint program)
{
	GLuint error = 0;
	SetUniformInt(lightCountVSID, light_list.size());
	SetUniformInt(lightCountFSID, light_list.size());

	for (int i = 0; i < light_list.size(); i++)
	{
		PointLight point = light_list.at(i);
		SetUniformVector3(lightPositionID[i], point.position);
		SetUniformVector3(lightColorID[i], point.color);
		//SetUniformVector3(attenuationID[i], point.attenuation);
	}
}

void PointLightFX::Add(PointLight& point)
{
	if (light_list.size() + 1 > MAX_POINT_LIGHT_COUNT)
		return;

	light_list.push_back(point);
}
