#include "fog_fx.h"

FogFX::FogFX() 
	: CommonShaderBase(),
	VS_enableID(0), FS_enableID(0),
	sky_colorID(0), enable(false),
	density(0.0007), gradient(1.5),
	densityID(0), gradientID(0)
{
	sky_color = glm::vec3(1,1,1);
}

FogFX::~FogFX()
{}

void FogFX::Load(GLuint program)
{
	VS_enableID = glGetUniformLocation(program, "fog_vs_enable");
	densityID = glGetUniformLocation(program, "density");
	gradientID = glGetUniformLocation(program, "gradient");

	FS_enableID = glGetUniformLocation(program, "fog_fs_enable");
	sky_colorID = glGetUniformLocation(program, "fog_skycolor");
}

void FogFX::Use(GLuint program)
{
	SetUniformInt(VS_enableID, enable ? 1 : 0);
	SetUniformFloat(densityID, density);
	SetUniformFloat(gradientID, gradient);
	SetUniformVector3(sky_colorID, sky_color);
	SetUniformInt(FS_enableID, enable ? 1 : 0);
}


void FogFX::SetEnable(bool flag)
{
	enable = flag;
}

void FogFX::SetColor(float r, float g, float b)
{
	sky_color = glm::vec3(r, g, b);
}

void  FogFX::SetDensity(float d)
{
	density = d;
}

void  FogFX::SetGradient(float g)
{
	gradient = g;
}