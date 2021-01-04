#pragma once
#include "scene_object.h"
#include "assimp_mesh.h"
#include "graphics/FBO.h"
#include "GLShaderfx.h"

class RavineLevel : SceneObject
{
public:
	RavineLevel();
	~RavineLevel();

	void Initialize();

	virtual void Update(float elapsed);
	void Draw(glm::mat4 viewProjection);

private:
	AssimpMesh	_baselevel;
	AssimpMesh	_crystal;
	FBO			_color1;
	FBO			_color2;
	FBO			_pingPong[2];
	FBO			_finalScene;
	FBORectangle _screenRect;
	GLShaderFx	_blurShader;
	GLShaderFx	_bloomFinal;
	GLShaderFx	_modelShader;
	GLShaderFx	_shaderOrtho;
	GLShaderFx	_brightFilter;
	float		_alpha;
	int			_alpha_dir;

};

