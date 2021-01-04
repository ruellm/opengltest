#pragma once

#include "common.h"

class FBO
{
public:
	FBO();
	virtual ~FBO();

	int		Initialize(int width, int height, bool createDepthStencil=false);
	void	Bind();
	void	Unbind(int width, int height);
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
};

