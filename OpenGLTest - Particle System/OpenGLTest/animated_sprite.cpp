#include "animated_sprite.h"

#define SUPPORT_VAO		1

AnimatedSprite::AnimatedSprite()
{
	_frameCount = 1.0;
	_currentFrame = 0;
	_frameWidth = 0;
	_bLoop = true;
	_called = false;
	_handler = NULL;
	_VBO = 0;
	_VAO = 0;
}


AnimatedSprite::~AnimatedSprite()
{
	if (_VBO) {
		glDeleteBuffers(1, &_VBO);
	}

#if SUPPORT_VAO //Opengl es 3.0
	if (_VAO) {
		glDeleteVertexArrays(1, &_VAO);
	}
#endif
}

void AnimatedSprite::Load(const char* path)
{
	_texture.LoadFromAssetDirect(path);

	// for 3D, it is always better that 0,0,0 is in the center
#if 1
	float vertices[] = {
		// 3D Coordinates	texture coordinates, 
		// in windows, image is upside down, therefore invert texture coordinates
		1.0f,  1.0f, 0.0f,		1.0f, 0.0f,		// top right
		1.0f, -1.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-1.0f,  1.0f, 0.0f,		0.0f, 0.0f,		// top left 
												// second triangle
		1.0f, -1.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-1.0f, -1.0f, 0.0f,		0.0f, 1.0f,		// bottom left
		-1.0f,  1.0f, 0.0f,		0.0f, 0.0f		// top left
	};
#endif
	
#if SUPPORT_VAO //opengl es 3.0
	glGenVertexArrays(1, &_VAO);
	glGenBuffers(1, &_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(_VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float)));  

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#else

	glGenBuffers(1, &_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#endif
}

void AnimatedSprite::Set(int frame_count, int fps, bool loop, int frameWidth)
{
	SetLoop(loop);
	SetFPS(fps);
	_frameCount = frame_count;

	_frameWidth = frameWidth;
}

void AnimatedSprite::SetLoop(bool flag)
{
	_bLoop = flag;
}

void AnimatedSprite::SetFPS(int fps)
{
	m_timeBetweenFrames = 1.0f / (float)fps;
	m_timeSinceLastFrame = m_timeBetweenFrames;
}

void AnimatedSprite::Reset()
{
	if (_currentFrame == _frameCount - 1) {
		_currentFrame = 0;
		_called = false;
	}
}

int AnimatedSprite::GetFrameWidth()
{
	return _frameWidth;
}

float AnimatedSprite::GetFrameWidthNormalized()
{
	return _frameWidth / (float)_texture._width;
}

int AnimatedSprite::GetFrameCount()
{
	return _frameCount;
}

int AnimatedSprite::GetCurrentFrame()
{
	return _currentFrame;
}

void AnimatedSprite::Update(float elapsed)
{
	bool update = false;

	m_timeSinceLastFrame -= elapsed;
	if (m_timeSinceLastFrame <= 0) {
		m_timeSinceLastFrame = m_timeBetweenFrames;
		update = true;
	}

	if (update) {
		++_currentFrame;
		if (_bLoop) {
			_currentFrame %= _frameCount;
		}
		else if (_currentFrame >= _frameCount) {
			_currentFrame = _frameCount - 1;
			if (_handler) {
				if (_bLoop == false && _called == false) {
					_called = true;
					_handler->OnAnimationComplete(this);
				}
			}
		}
	}
}

void AnimatedSprite:: Draw()
{
	GLenum error;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
	_texture.Bind();

#if SUPPORT_VAO //Opengl es 3.0
	glBindVertexArray(_VAO);
	error = glGetError();

#else

	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)&vertices[3]); // 3 * sizeof(float)
#endif
	glDrawArrays(GL_TRIANGLES, 0, 6);
	error = glGetError();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
}

void AnimatedSprite::SetCallbackHandler(ISpriteCallback* callback)
{
	_handler = callback;
}
