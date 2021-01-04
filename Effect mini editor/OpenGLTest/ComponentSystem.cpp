#include "ComponentSystem.h"
#include "effects/particle_system.h"
#include "graphics/glshaderfx.h"

using namespace ramo::effects;
using namespace ramo::graphics;

static GLShaderFx particleSystem;
static GLShaderFx meshShader;
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
}

glm::mat4 GetProjectionMatrix()
{
	return projectionMatrix;
}


void UpdateObject(float elapsed, GameObject* object)
{
	if (!object->IsVisible()) return;

	if (object->CheckComponent(GAMEOBJ_COMPONENT_CAMERAPOS))
		object->SetCameraPos(camera.GetPosition());

	object->Update(elapsed, glm::mat4(1.0f));
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
	int worldPos = shader->GetUniformLocation("WorldPos");
	int gVP = shader->GetUniformLocation("gVP");
	int textureID = shader->GetUniformLocation("gSampler");
	int alphaID = shader->GetUniformLocation("alpha");
	int glow = shader->GetUniformLocation("glow");


	int positionID = shader->GetAttributeLocation("Position");
	int textCoord = shader->GetAttributeLocation("vTexCoord");
	int normal = shader->GetAttributeLocation("vNormal");

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
	shader->SetUniformMatrix(worldPos, world);

	//billboard setup
	int cameraRightWorldID = shader->GetUniformLocation("CameraRight_worldspace");
	int cameraUpWorldID = shader->GetUniformLocation("CameraUp_worldspace");

	shader->SetUniformInt(glow, obj->IsGlow() ? 1 : 0);
	shader->SetUniformVector3(cameraUpWorldID, glm::vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]));
	shader->SetUniformVector3(cameraRightWorldID, glm::vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]));

	int scaleW = shader->GetUniformLocation("scaleW");
	int scaleH = shader->GetUniformLocation("scaleH");

	shader->SetUniformFloat(scaleW, particle->_scaleW);
	shader->SetUniformFloat(scaleH, particle->_scaleH);

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

	//	int textureID = shader->GetUniformLocation("gSampler");
	//	glUniform1i(textureID, 0);
	//	glActiveTexture(GL_TEXTURE0);

	glm::mat4 model = glm::mat4(1.0f);
	model = obj->GetWorldMatrix();

//	glm::mat4 projectionMatrix = Engine::GetGraphics()->GetProjectionMatrix();
	glm::mat4 viewMatrix = camera.GetViewMatrix();

	glm::mat4 WVP = projectionMatrix * viewMatrix * model;
	//shader->SetUniformMatrix(gWVP, WVP);
//	shader->SetUniformMatrix(gWorld, model);
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
