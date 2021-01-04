// ComponentSystem.cpp
// An independent component system used for unit testing
// Created 3-29-2019
// author: ruellm@yahoo.com
#pragma once
#include "scene/game_object.h"
#include "scene/camera.h"

extern Camera camera;

void InitializeComponentSystem();
void UpdateObject(float elapsed, GameObject* object, glm::mat4 matrix);
void DrawObject(GameObject* obj);
glm::mat4 GetProjectionMatrix();


//For shadow map
void SetShadowMVP(glm::mat4& view, glm::mat4& proj);
void DrawObjectShadowMap(GameObject* obj);
void DrawMeshShadow(GameObject* obj);
void SetShadowTexture(GLuint texture);
void SetDirectional(glm::vec3 dir);

