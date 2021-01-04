// ComponentSystem.cpp
// An independent component system used for unit testing
// Created 3-29-2019
// author: ruellm@yahoo.com
#pragma once
#include "scene/game_object.h"
#include "scene/camera.h"

extern Camera camera;

void InitializeComponentSystem();
void UpdateObject(float elapsed, GameObject* object);
void DrawObject(GameObject* obj);
glm::mat4 GetProjectionMatrix();
