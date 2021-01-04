//-----------------------------------------------
// Render to Texture demo - can be use as temporary buffer
// source referencese:
//  https://open.gl/framebuffers
//  http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
//  https://software.intel.com/en-us/articles/dynamic-resolution-rendering-on-opengl-es-2
//  Created: ruellm@yahoo.com
//  September 09, 2018
//  adapted from https://www.dropbox.com/sh/2c6ife8vt5p4eh4/AABfnnXrOKREcolbHwsqH2Bha/postProcessing?dl=0&preview=Fbo.java&subfolder_nav_tracking=1

#include "FBO.h"

FBO::FBO() 
	: _width(0)
    , _height(0)
	, _frameBuffer(0)
	, _texColorBuffer(0)
	, _rboDepthStencil(0)
{
	//...
}


FBO::~FBO()
{
	Cleanup();
}

int	FBO::Initialize(int width, int height, bool createDepthStencil)
{
	int error = 0;

	_width = width;
	_height = height;

	// 1. create frame buffer
	glGenFramebuffers(1, &_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

	// 2. create a blank texture which will contain the RGB output of our shader.
	// data is set to NULL
	glGenTextures(1, &_texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, _texColorBuffer);

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
	);

	error = glGetError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 3. attached our texture to the frame buffer, note that our custom frame buffer is already active
	// by glBindFramebuffer
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texColorBuffer, 0
	);

	error = glGetError();

	// 4. we create the depth and stencil buffer also, (this is optional)
	if (createDepthStencil) {
		GLuint rboDepthStencil;
		glGenRenderbuffers(1, &rboDepthStencil);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height);
	}

	error = glGetError();

	// Set the list of draw buffers. this is not needed?
	//GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
	error = glGetError();
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		return -1;
	}
	// Restore frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return glGetError();;
}

void FBO::Cleanup()
{
	if(_frameBuffer)
		glDeleteFramebuffers(1, &_frameBuffer);
	if(_texColorBuffer)
		glDeleteTextures(1, &_texColorBuffer);
	if(_rboDepthStencil)
		glDeleteRenderbuffers(1, &_rboDepthStencil);
}

void FBO::Bind()
{
	// Render to our frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	glViewport(0, 0, _width, _height);						// use the entire texture,
															// this means that use the dimension set as our total 
															// display area
}

void FBO::Unbind(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

int FBO::GetColorTexture()
{
	return _texColorBuffer;
}


int	FBO::GetWidth()
{
	return _width;
}

int	FBO::GetHeight()
{
	return _height;
}