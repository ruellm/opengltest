#include "ComponentSystem.h"
#include "effects/particle_system.h"
#include "graphics/glshaderfx.h"

using namespace ramo::effects;
using namespace ramo::graphics;

static GLShaderFx particleSystem;
static GLShaderFx meshShader;
static GLShaderFx shadowDepthShader;
static glm::mat4  shadowViewMat = glm::mat4(1.0f);
static glm::mat4  shadowProjMat = glm::mat4(1.0f);
static glm::vec3  directionalLightDir = glm::vec3(0,0,0);
static GLuint shadowDepthTexture = 0;

Camera camera;
static glm::mat4 projectionMatrix = glm::perspective(glm::radians(60.0f),
	4.0f / 3.0f, 1.0f, 1000.0f);

void DrawParticle(GameObject* obj);
void DrawMesh(GameObject* obj, GLShaderFx* shader = NULL);

void InitializeComponentSystem()
{
	particleSystem.Init();
	particleSystem.Load("shaders/particle_shader_instance.vs", GL_VERTEX_SHADER);
	particleSystem.Load("shaders/particle_shader.fs", GL_FRAGMENT_SHADER);
	particleSystem.Build();

	int possition_attribute = particleSystem.GetAttributeLocation("vPosition");
	int texture_attribute = particleSystem.GetAttributeLocation("vTexCoord");

	int possition_2_attribute = particleSystem.GetAttributeLocation("vPosition_2");
	int color_attribute = particleSystem.GetAttributeLocation("vColor");
	int size_attribute = particleSystem.GetAttributeLocation("fSize");

	ramo::effects::ParticleSystem::POSITION_ATTRIBUTE = possition_attribute;
	ramo::effects::ParticleSystem::TEXTCOORD_ATTRIBUTE = texture_attribute;
	ramo::effects::ParticleSystem::POSITION_2_ATTRIBUTE = possition_2_attribute;
	ramo::effects::ParticleSystem::COLOR_ATTRIBUTE = color_attribute;
	ramo::effects::ParticleSystem::SIZE_ATTRIBUTE = size_attribute;

	meshShader.Init();
	meshShader.Load("shaders/skinning.vs", GL_VERTEX_SHADER);
	meshShader.Load("shaders/model_fshader_no_common", GL_FRAGMENT_SHADER);
	meshShader.Build();
	meshShader.Use();

	int positionID = meshShader.GetAttributeLocation("Position");
	int textCoord = meshShader.GetAttributeLocation("vTexCoord");
	int normal = meshShader.GetAttributeLocation("vNormal");

	int boneids = meshShader.GetAttributeLocation("BoneIDs");
	int weights = meshShader.GetAttributeLocation("Weights");

	ramo::graphics::AssimpMesh::SetAttributeLocation(positionID, textCoord, normal, boneids, weights);

	shadowDepthShader.Init();
	shadowDepthShader.Load("shaders/skinning.vs", GL_VERTEX_SHADER);
	shadowDepthShader.Load("shaders/shadowmap_depth_fs", GL_FRAGMENT_SHADER);
	shadowDepthShader.Build();
	shadowDepthShader.Use();
	
}

glm::mat4 GetProjectionMatrix()
{
	return projectionMatrix;
}


void UpdateObject(float elapsed, GameObject* object, glm::mat4 parentMatrix)
{
	if (!object->IsVisible()) return;

	if (object->CheckComponent(GAMEOBJ_COMPONENT_CAMERAPOS))
		object->SetCameraPos(camera.GetPosition());

	object->Update(elapsed, parentMatrix);

	for (int i = 0; i < object->m_children.size(); i++) {
		if (object->m_children.at(i)->IsVisible()) {
			GameObject* child = object->m_children.at(i);
			UpdateObject(elapsed, child, object->GetWorldMatrix());
		}
	}
}

void DrawObject(GameObject* obj)
{
	if (!obj->IsVisible()) return;

	if (obj->CheckComponent(GAMEOBJ_COMPONENT_PARTICLE))
	{
		DrawParticle(obj);
	}

	if (obj->CheckComponent(GAMEOBJ_COMPONENT_MESH))
	{
		DrawMesh(obj);
	}

#if 0

	if (obj->CheckComponent(GAMEOBJ_COMPONENT_ANIMATED_SPRITE))
	{
		DrawAnimatedSprite(obj);
	}

#endif

	if (obj->CheckComponent(GAMEOBJ_COMPONENT_CUSTOM_MESH)) {
		glm::mat4 viewMatrix = camera.GetViewMatrix();

		CustomMeshDrawParam param;
		param.viewMatrix = viewMatrix;
		param.projectionMatrix = projectionMatrix;
		param.worldMatrix = obj->GetWorldMatrix();
		param.alpha = obj->GetAlpha();
		param.isDarken = obj->IsDarken();
		param.isGlow = obj->IsGlow();

		obj->GetCustomMesh()->Draw(param);
	}


end:
	for (int i = 0; i < obj->m_children.size(); i++)
	{
		DrawObject(obj->m_children.at(i));
	}
}


void DrawParticle(GameObject* obj)
{
	ParticleSystem* particle = obj->GetParticle();
	if (particle == 0) return;

	GLShaderFx* shader = &particleSystem;
	shader->Use();
#if 1
	int worldMatrixID = shader->GetUniformLocation("WorldMatrix");
	int worldPosID = shader->GetUniformLocation("WorldPos");
	int gVP = shader->GetUniformLocation("gVP");
	int textureID = shader->GetUniformLocation("gSampler");
	int alphaID = shader->GetUniformLocation("alpha");
	int glow = shader->GetUniformLocation("glow");


	if (obj->CheckComponent(GAMEOBJ_COMPONENT_ALPHA)) {
		shader->SetUniformFloat(alphaID, obj->GetAlpha());
	}
	else {
		shader->SetUniformFloat(alphaID, 1.0f);
	}

	glm::mat4 viewMatrix = camera.GetViewMatrix();
	glm::mat4 VP = projectionMatrix * viewMatrix;  // <-- this cause a problem sometimes
	shader->SetUniformMatrix(gVP, VP);

	glm::mat4 world = obj->GetWorldMatrix();
	glm::vec3 worldPos= glm::vec3(
		world[3][0],
		world[3][1],
		world[3][2]);


	//shader->SetUniformVector3(worldPosID, worldPos);
//	shader->SetUniformMatrix(worldMatrixID, world);
	shader->SetUniformMatrix(worldPosID, world);
	shader->SetUniformInt(glow, obj->IsGlow() ? 1 : 0);
	
	int renderAlignID = shader->GetUniformLocation("renderAlign");
	if (particle->_renderAlign == PARTICLE_RENDER_ALIGN_BILLBOARD)
	{
		shader->SetUniformInt(renderAlignID, 0);
		//billboard setup
		int cameraRightWorldID = shader->GetUniformLocation("CameraRight_worldspace");
		int cameraUpWorldID = shader->GetUniformLocation("CameraUp_worldspace");

		shader->SetUniformVector3(cameraUpWorldID, glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));
		shader->SetUniformVector3(cameraRightWorldID, glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));
	}
	else {
		shader->SetUniformInt(renderAlignID, 1);

		glm::mat4 matRotX = glm::mat4(1.0f);
		matRotX = glm::rotate(matRotX, particle->_renderAlignRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::mat4 matRotY = glm::mat4(1.0f);
		matRotY = glm::rotate(matRotY, particle->_renderAlignRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 matRotZ = glm::mat4(1.0f);
		matRotZ = glm::rotate(matRotZ, particle->_renderAlignRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 rotMat = glm::mat4(1.0f);
		rotMat = matRotX * matRotY * matRotZ;
		
		int rotmatid = shader->GetUniformLocation("rotateMat");
		shader->SetUniformMatrix(rotmatid, rotMat);
	}

	int scaleW = shader->GetUniformLocation("stretchedScale");
	glm::mat4 scaleMat = glm::mat4(1.0f);
	shader->SetUniformVector2(scaleW, glm::vec2(particle->_scaleW, particle->_scaleH));

	glUniform1i(textureID, 0);				// set the textureID to texture unit 0
	glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture

	particle->Render(shader);
#endif

}

void DrawMesh(GameObject* obj, GLShaderFx* shader)
{
	AssimpMesh* mesh = obj->GetMesh();
	if (mesh == 0) return;

	shader = &meshShader;
	shader->Use();

	///----------------------------------------------------------
//	int gWVP = shader->GetUniformLocation("gWVP");
	//int gWorld = shader->GetUniformLocation("gWorld");

	// for shader uniforms
	int viewID = shader->GetUniformLocation("viewMatrix");
	int projectionID = shader->GetUniformLocation("projectionMatrix");
	int gWorld = shader->GetUniformLocation("worldMatrix");
	int darken = shader->GetUniformLocation("darken");
	int glow = shader->GetUniformLocation("glow");

	// for shader attributes
	int positionID = shader->GetAttributeLocation("Position");
	int textCoord = shader->GetAttributeLocation("vTexCoord");
	int normal = shader->GetAttributeLocation("vNormal");

	int boneids = shader->GetAttributeLocation("BoneIDs");
	int weights = shader->GetAttributeLocation("Weights");

	int directionalLightID = shader->GetUniformLocation("directionalLight");
	int lightVPMatrixID = shader->GetUniformLocation("lightSpaceMatrix");
	
	
	int shadowMapID = shader->GetUniformLocation("shadowMap");
	glUniform1i(shadowMapID, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);

	glm::mat4 model = glm::mat4(1.0f);
	model = obj->GetWorldMatrix();

	//	glm::mat4 projectionMatrix = Engine::GetGraphics()->GetProjectionMatrix();
	glm::mat4 viewMatrix = camera.GetViewMatrix();
	//glm::mat4 projMatrix = glm::ortho<float>(-30, 30, -30, 30, -30, 100);;

	//glm::mat4 WVP = projectionMatrix * viewMatrix * model;
	//shader->SetUniformMatrix(gWVP, WVP);
	//shader->SetUniformMatrix(gWorld, model);
	shader->SetUniformMatrix(viewID, viewMatrix);
	shader->SetUniformMatrix(projectionID, projectionMatrix);
	shader->SetUniformMatrix(gWorld, model);

	int gBones[50];
	for (int b = 0; b < mesh->BoneTransforms.size(); b++) {
		char format[MAX_BONES];
		sprintf(format, "gBones[%d]", b);
		gBones[b] = shader->GetUniformLocation(format);

		// set bon transform temporary
		glm::mat4 matrix = mesh->BoneTransforms.at(b);
		shader->SetUniformMatrix(gBones[b], matrix);
	}

	int withBonesID = shader->GetUniformLocation("withBones");
	shader->SetUniformInt(withBonesID, (mesh->IsAnimated()) ? 1 : 0);

	// alpha
	int alphaID = shader->GetUniformLocation("alpha");
	if (obj->CheckComponent(GAMEOBJ_COMPONENT_ALPHA)) {
		shader->SetUniformFloat(alphaID, obj->GetAlpha());
	}
	else
		shader->SetUniformFloat(alphaID, 1.0f);

	shader->SetUniformInt(darken, obj->IsDarken() ? 1 : 0);
	shader->SetUniformMatrix(gWorld, model);
	shader->SetUniformInt(glow, obj->IsGlow() ? 1 : 0);

	// temporary directional environment light
	shader->SetUniformVector3(directionalLightID,directionalLightDir);

	shader->SetUniformMatrix(lightVPMatrixID, shadowProjMat * shadowViewMat);
	// set shaders attribute
	obj->GetMesh()->SetAttributeLocation(positionID,
		textCoord, normal, boneids, weights);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int textureID = shader->GetUniformLocation("gSampler");
	glUniform1i(textureID, 0);
	glActiveTexture(GL_TEXTURE0);

	obj->PreMeshRender();

	obj->GetMesh()->Render();

	obj->PostMeshRender();

	glDisable(GL_BLEND);
}


/**/
// Shadow map rendering
/**/
void DrawObjectShadowMap(GameObject* obj)
{
	if (!obj->IsVisible()) return;

	if (obj->CheckComponent(GAMEOBJ_COMPONENT_MESH))
	{
		DrawMeshShadow(obj);
	}

	if (obj->CheckComponent(GAMEOBJ_COMPONENT_CUSTOM_MESH)) {
		glm::mat4 viewMatrix = camera.GetViewMatrix();

		CustomMeshDrawParam param;
		param.viewMatrix = viewMatrix;
		param.projectionMatrix = projectionMatrix;
		param.worldMatrix = obj->GetWorldMatrix();
		param.alpha = obj->GetAlpha();
		param.isDarken = obj->IsDarken();
		param.isGlow = obj->IsGlow();

		obj->GetCustomMesh()->Draw(param);
	}

end:
	for (int i = 0; i < obj->m_children.size(); i++)
	{
		DrawObjectShadowMap(obj->m_children.at(i));
	}
}

void DrawMeshShadow(GameObject* obj)
{
	AssimpMesh* mesh = obj->GetMesh();
	if (mesh == 0) return;

	GLShaderFx* shader = &shadowDepthShader;
	shader->Use();

	///----------------------------------------------------------
	// for shader uniforms
	int viewID = shader->GetUniformLocation("viewMatrix");
	int projectionID = shader->GetUniformLocation("projectionMatrix");
	int gWorld = shader->GetUniformLocation("worldMatrix");
	int darken = shader->GetUniformLocation("darken");
	int glow = shader->GetUniformLocation("glow");

	// for shader attributes
	int positionID = shader->GetAttributeLocation("Position");
	int textCoord = shader->GetAttributeLocation("vTexCoord");
	int normal = shader->GetAttributeLocation("vNormal");

	int boneids = shader->GetAttributeLocation("BoneIDs");
	int weights = shader->GetAttributeLocation("Weights");

//	int directionalLightID = shader->GetUniformLocation("directionalLight");

	int lightVPMatrixID = shader->GetUniformLocation("lightSpaceMatrix");
	shader->SetUniformMatrix(lightVPMatrixID, glm::mat4(1.0f));

	glm::mat4 model = glm::mat4(1.0f);
	model = obj->GetWorldMatrix();

	shader->SetUniformMatrix(viewID, shadowViewMat);
	shader->SetUniformMatrix(projectionID, shadowProjMat);
	shader->SetUniformMatrix(gWorld, model);

	int gBones[50];
	for (int b = 0; b < mesh->BoneTransforms.size(); b++) {
		char format[MAX_BONES];
		sprintf(format, "gBones[%d]", b);
		gBones[b] = shader->GetUniformLocation(format);

		// set bon transform temporary
		glm::mat4 matrix = mesh->BoneTransforms.at(b);
		shader->SetUniformMatrix(gBones[b], matrix);
	}

	int withBonesID = shader->GetUniformLocation("withBones");
	shader->SetUniformInt(withBonesID, (mesh->IsAnimated()) ? 1 : 0);

	// alpha
	int alphaID = shader->GetUniformLocation("alpha");
	if (obj->CheckComponent(GAMEOBJ_COMPONENT_ALPHA)) {
		shader->SetUniformFloat(alphaID, obj->GetAlpha());
	}
	else
		shader->SetUniformFloat(alphaID, 1.0f);

	shader->SetUniformInt(darken, obj->IsDarken() ? 1 : 0);
	shader->SetUniformMatrix(gWorld, model);
	shader->SetUniformInt(glow, obj->IsGlow() ? 1 : 0);

	// set shaders attribute
	obj->GetMesh()->SetAttributeLocation(positionID,
		textCoord, normal, boneids, weights);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	obj->PreMeshRender();

	obj->GetMesh()->Render();

	obj->PostMeshRender();

	glDisable(GL_BLEND);
}


void SetShadowMVP(glm::mat4& view, glm::mat4& proj)
{
	shadowViewMat = view;
	shadowProjMat = proj;
}

void SetShadowTexture(GLuint texture)
{
	shadowDepthTexture = texture;
}

void SetDirectional(glm::vec3 dir)
{
	directionalLightDir = dir;
}