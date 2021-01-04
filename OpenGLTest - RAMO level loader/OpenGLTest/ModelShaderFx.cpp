#include "stdafx.h"
#include "ModelShaderFx.h"


ModelShaderFx::ModelShaderFx(void) : 
	m_VP(0), m_sampler(0)
{
}


ModelShaderFx::~ModelShaderFx(void)
{
}

void ModelShaderFx::Load()
{
	Init();
	LoadVertexShaderFromFile("shaders/model_vshader.txt");
	LoadFragmentShaderFromFile("shaders/model_fshader.txt");
	Build();
	Use();

	m_VP = GetUniformLocation("gVP");
	if(  0xFFFFFFFF == m_VP) {
		//Log::Dbg("Error getting gVP object");
	}/**/

	m_sampler = GetUniformLocation("gSampler");
	if(  0xFFFFFFFF == m_VP) {
		//Log::Dbg("Error getting gBaseSampler object");
	}/**/

	m_World = GetUniformLocation("gWorld");
	if(  0xFFFFFFFF == m_VP) {
		//Log::Dbg("Error getting gVP object");
	}/**/

	LoadIncludes();
}

void ModelShaderFx::SetVPMatrix(glm::mat4& vp)
{
	SetUniformMatrix( m_VP, vp );
}

void ModelShaderFx::SetWorldMatrix(glm::mat4 world)
{
	SetUniformMatrix( m_World, world );
}

void ModelShaderFx::ActivateSampler()
{
	glUniform1i(m_sampler, 0);
}