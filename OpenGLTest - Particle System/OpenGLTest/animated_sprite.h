// Ported from SDLGX engine which in turn ported from 
// Nightmare Javascript Engine
// Author: Ruell Magpayo <ruellm@yahoo.com>
// Created Oct 01, 2012 ported Aug 8,2016 (SDLGX)
// Ported to RAMO/Phoenix/OpenGL Oct 20, 2017
// Animator class no longer exist, merged to this class
//
// Note: Shaders will be managed outside of this class to promote flexibility
// projection in shaders should be passed as VP instead.
// 1. to display spritesheet in 2D, pass orthographic as projection to shaders
// 2. to display spritesheet in 3D, pass a 3D projection and a camera to VP shaders

#pragma once
#include "texture2D.h"

//forward declaration
class AnimatedSprite;

class ISpriteCallback {
public:
	virtual void OnAnimationComplete(AnimatedSprite* sprite) = 0;
};

class AnimatedSprite
{
public:
	AnimatedSprite();
	virtual ~AnimatedSprite();

	void Load(const char* path);

	void SetFPS(int fps);
	void SetLoop(bool flag);
	void Set(int frame_count, int fps, bool loop, int frameWidth);			
	void SetCallbackHandler(ISpriteCallback* callback);

	int GetFrameWidth();
	int GetFrameCount();
	int GetCurrentFrame();
	float GetFrameWidthNormalized();

	virtual void Reset();
	virtual void Update(float elapsed);
	virtual void Draw();
protected:

	// Total animation frames
	int _frameCount;

	// current frame
	int _currentFrame;

	// animation frame width
	int _frameWidth;

	// the loop flag, if this animation should loop
	bool _bLoop;
	
	bool _called;

	ISpriteCallback* _handler;


private:

	// Adapted from animator class
	float m_timeBetweenFrames;
	float m_timeSinceLastFrame;
	Texture2D _texture;
	GLuint _VBO;
	GLuint _VAO;
};

