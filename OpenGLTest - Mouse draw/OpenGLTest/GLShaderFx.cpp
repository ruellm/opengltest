#include <stdio.h>
#include "GLShaderFx.h"

//ZBU_UNSURE : what shader version is supported in OpenGL ES 2?
#define SHADER_VERSION	("#version 330")


void CommonShaderBase::LoadShaderFromFile(const char* szFile)
{
	FILE* fp = fopen(szFile, "r");
	fseek(fp, 0, SEEK_END);
	unsigned long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* szShaderSrc = new char[size+1];
	memset(szShaderSrc, 0, size);
	fread(szShaderSrc, size, 1, fp);
	szShaderSrc[size] = '\0';

	fclose(fp);
	
	m_strCode = std::string(szShaderSrc);
}

//...

GLShaderFx::GLShaderFx(void)
{
	//...
}


GLShaderFx::~GLShaderFx(void)
{
	//...
	//glDeleteShader(VertexShaderID);
    //glDeleteShader(FragmentShaderID);
}

void GLShaderFx::Init()
{
	m_program = glCreateProgram();
}

void GLShaderFx::LoadVertexShaderFromFile(const char* file)
{
	m_vertexShader = LoadShaderFromFile(file, GL_VERTEX_SHADER);
	glAttachShader(m_program, m_vertexShader);
}

void GLShaderFx::LoadFragmentShaderFromFile(const char* file)
{
	m_fragmentShader = LoadShaderFromFile(file, GL_FRAGMENT_SHADER);
	glAttachShader(m_program, m_fragmentShader);
}

void GLShaderFx::Build()
{
	glLinkProgram(m_program);
	GLenum error = glGetError();

	// Get LINKING status
	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	glGetProgramiv(m_program, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(m_program, sizeof(ErrorLog), NULL, ErrorLog);

		//Log::Dbg("Error linking shader program: '%s'\n", ErrorLog);
		printf("Error linking shader program: '%s'\n", ErrorLog);
	}
}

void GLShaderFx::Use()
{
	glUseProgram(m_program);

	for(int i = 0; i < m_vsInclude.size(); i++ ) {
		CommonShaderBase* shader = m_vsInclude[i];
		shader->Use(m_program);
	}

	for(int i = 0; i < m_fsInclude.size(); i++ ) {		
		CommonShaderBase* shader = m_fsInclude[i];
		shader->Use(m_program);
	}
}

GLuint GLShaderFx::LoadShaderFromFile(const char* file, GLenum type)
{
	unsigned long size = 0;
	int ret = 0;
	GLuint shader = 0;

	FILE* fp = fopen(file, "r");
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* szShaderSrc = new char[size+1];
	memset(szShaderSrc, 0, size);
	fread(szShaderSrc, size, 1, fp);
	szShaderSrc[size] = '\0';

	//**
	shader = LoadShader( (const char*)szShaderSrc, type, size );
	//**

	delete[] szShaderSrc;
	if(fp) {
		fclose(fp);
	}

	return shader;
}

GLuint GLShaderFx::LoadShader(const char* szShaderSrc, GLenum type, int size)
{
	GLuint shader = 0;

	// Create the shader object
	shader = glCreateShader(type);
	
	std::string strTotalShader("");
	std::string srcShader(szShaderSrc);

	// append include files
	if( type == GL_VERTEX_SHADER ) {
		for(int i = 0; i < m_vsInclude.size(); i++ ) {
			CommonShaderBase* shader = m_vsInclude[i];
			strTotalShader += shader->GetShaderCode();
		}
	} else if (type == GL_FRAGMENT_SHADER) {
		for(int i = 0; i < m_fsInclude.size(); i++ ) {		
			CommonShaderBase* shader = m_fsInclude[i];
			strTotalShader += shader->GetShaderCode();
		}
	}

	std::string version(SHADER_VERSION);
	strTotalShader = version + "\r\n" + strTotalShader;
	strTotalShader += srcShader;
	const char* szShader = strTotalShader.c_str();
	
	// Load the shader source
	if( size ) {
		size = strTotalShader.size();
		glShaderSource(shader,1, &szShader, &size);
	} else {
		glShaderSource(shader,1, &szShader, NULL);
	}

	// Compile the shader
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(shader, sizeof(InfoLog), NULL, InfoLog);
	
		//Log::Dbg("%s \n", InfoLog);
		printf("%s \n", InfoLog);

#if _DEBUG
		FILE* fp = fopen("error_shader.txt","w");
		fwrite(szShader, strlen(szShader), 1, fp);
		fclose(fp);
#endif
	}

	return shader;
}

int GLShaderFx::GetUniformLocation(const char* variable)
{
	return glGetUniformLocation(m_program, variable);
}

void GLShaderFx::SetUniformMatrix( int uniformid, glm::mat4& matrix)
{
	glUniformMatrix4fv(uniformid, 1, GL_FALSE, glm::value_ptr(matrix));
}

void GLShaderFx::SetUniformVector3( int uniformid, glm::vec3& value )
{
		glUniform3f(uniformid, value.x, value.y, value.z);
}

void GLShaderFx::SetUniformVector2(int uniformid, glm::vec2& value)
{
	glUniform2f(uniformid, value.x, value.y);
}

//........................................................

void GLShaderFx::vsInclude(CommonShaderBase* shader)
{
	m_vsInclude.push_back(shader);
}

void GLShaderFx::fsInclude(CommonShaderBase* shader)
{
	m_fsInclude.push_back(shader);
}

void GLShaderFx::LoadIncludes( )
{
	for(int i = 0; i < m_vsInclude.size(); i++ ) {
		CommonShaderBase* shader = m_vsInclude[i];
		shader->Load(m_program);
	}

	for(int i = 0; i < m_fsInclude.size(); i++ ) {		
		CommonShaderBase* shader = m_fsInclude[i];
		shader->Load(m_program);
	}
}


