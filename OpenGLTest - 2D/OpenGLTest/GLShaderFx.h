#pragma once

#include "common.h"

// Advance Declaration
/* ... */
class CommonShaderBase
{
public:
	void LoadShaderFromFile(const char* szFile);
	virtual void Load(GLuint program) = 0;
    virtual void Use(GLuint program) = 0;

	inline std::string GetShaderCode() const {
		return m_strCode;
	}

protected:
	std::string m_strCode;
};


class GLShaderFx
{
public:
	GLShaderFx(void);
	virtual ~GLShaderFx(void);

	GLuint LoadShaderFromFile(const char* file, GLenum type);
	GLuint LoadShader(const char* szShaderSrc, GLenum type, int size = 0);

	void LoadVertexShaderFromFile(const char* file);
	void LoadFragmentShaderFromFile(const char* file);
	void Build();
	void Init();
	virtual void Use();

	int GetUniformLocation(const char* variable);
	void SetUniformMatrix( int uniformid, glm::mat4& matrix);
	void SetUniformVector3( int uniformid, glm::vec3& value );
	
	void vsInclude(CommonShaderBase* shader);
	void fsInclude(CommonShaderBase* shader);

	inline void SetProgram(GLuint program){
		m_program = program;
	}
	
	inline GLuint GetProgram() { 
		return m_program; 
	}

protected:
	GLuint	m_program;
	GLuint	m_vertexShader;
	GLuint	m_fragmentShader;

	std::vector<CommonShaderBase*> m_vsInclude;
	std::vector<CommonShaderBase*> m_fsInclude;

	void LoadIncludes( );
};

