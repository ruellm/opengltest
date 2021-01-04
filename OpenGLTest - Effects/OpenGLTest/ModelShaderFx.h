#pragma once

#include "common.h"
#include "GLShaderFx.h"

class ModelShaderFx : public GLShaderFx
{
public:
	ModelShaderFx(void);
	~ModelShaderFx(void);

	 void Load();
	 void SetVPMatrix(glm::mat4& vp);
	 void SetWorldMatrix(glm::mat4 world);
	 void ActivateSampler();
private:
	GLuint m_VP;
	GLuint m_World;
	GLuint m_sampler;
};

