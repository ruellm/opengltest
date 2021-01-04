#pragma once

#include "common.h"

class FBO
{
public:
	FBO();
	virtual ~FBO();

	int		Initialize(int width, int height, bool createDepthStencil=false);
	void	Bind();
	void	Unbind();
	int		GetColorTexture();
	void	Cleanup();

	int		GetWidth();
	int		GetHeight();

private:
	int		_width;
	int		_height;
	GLuint	_frameBuffer;
	GLuint	_texColorBuffer;
	GLuint  _rboDepthStencil;
	int backup_FBO;
	int _backupWidth;
	int _backupHeight;
};

/**/
class FBORectangle
{
public:
	FBORectangle();
	~FBORectangle();
	void Create();
	void Draw();

	int _position_attrib_id;
	int _texture_attrib_id;
private:
	GLuint _VAO;
	GLuint _VBO;

};