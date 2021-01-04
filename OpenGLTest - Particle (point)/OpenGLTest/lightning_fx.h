//
// Author: magpayor
// Created: 10/21/2017
//
#pragma once

#include "animated_sprite.h"
#include "scene_object.h"

class LightningBolt : public AnimatedSprite, public SceneObject
	//TODO: this will also inherit from SceneObject class
{
public:
	LightningBolt();
	~LightningBolt();
	virtual void Update(float elapsed);
	virtual void Draw();

	float GetAlpha();
	void SetCameraPos(glm::vec3 campos);
	void SetAlphaSpeed(float in, float out);
	void SetStaySeconds(float sec);

	void Show();
	int GetStatus();

private:
	float _alpha;
	float _alphaSpeedIn;
	float _alphaSpeedOut;
	float _staySeconds;
	glm::vec3 _campos;
};


