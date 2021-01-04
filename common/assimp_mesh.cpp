#include "assimp_mesh.h"

static glm::mat4 AiToGLMMat4(aiMatrix4x4& in_mat);
static glm::mat4 AiToGLMMat4(aiMatrix3x3& in_mat);

// TODO: load the attribute location
static int POSITION_LOCATION = 0;
static int TEX_COORD_LOCATION = 1;
static int NORMAL_LOCATION = 2;
static int BONE_ID_LOCATION = 3;
static int BONE_WEIGHT_LOCATION = 4;

void AssimpMesh::VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
{
	for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) {
		if (Weights[i] == 0.0) {
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	}
}

//
//
//

AssimpMesh::AssimpMesh() 
	: m_NumBones(0), m_animated(false), 
	_currentAnimationTime(0.0f)
{
	//...
}


AssimpMesh::~AssimpMesh()
{
}

void AssimpMesh::CleanUp()
{
	for (unsigned int i = 0; i < m_Textures.size(); i++) {
		SAFE_DELETE(m_Textures[i]);
	}

	for (unsigned int i = 0; i < m_Entries.size(); i++) {
		MeshEntry* entry = &m_Entries[i];
		if (entry->m_Buffers[0] != 0) {
			glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(entry->m_Buffers), entry->m_Buffers);
		}
	}
}

void AssimpMesh::SetAttributeLocation(
	int position,
	int texture,
	int normal,
	int boneID,
	int boneWeight)
{
	// TODO: load the attribute location
	POSITION_LOCATION = position;
	TEX_COORD_LOCATION = texture;
	NORMAL_LOCATION = normal;
	BONE_ID_LOCATION = boneID;
	BONE_WEIGHT_LOCATION = boneWeight;

}

bool AssimpMesh::IsAnimated()
{
	return m_animated;
}

bool AssimpMesh::Load(const std::string& Filename)
{
	CleanUp();

	m_pScene = m_Importer.ReadFile(Filename.c_str(), ASSIMP_LOAD_FLAGS);
	
	bool Ret = false;
	if (m_pScene) {
		m_GlobalInverseTransform = AiToGLMMat4(m_pScene->mRootNode->mTransformation);
		m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);
		Ret = InitFromScene(m_pScene, Filename);
	}
	else {
		printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
	}

	return Ret;
}


bool AssimpMesh::InitFromScene(const aiScene* pScene, const std::string& Filename)
{
	m_Entries.resize(pScene->mNumMeshes);
	m_Textures.resize(pScene->mNumMaterials);

	// Initialize the meshes in the scene one by one
	for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {

		m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_Entries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;

		const aiMesh* paiMesh = pScene->mMeshes[i];

		std::vector<glm::vec3> Positions;
		std::vector<glm::vec3> Normals;
		std::vector<glm::vec2> TexCoords;
		std::vector<unsigned int> Indices;
		std::vector<VertexBoneData> Bones;

		Bones.resize(pScene->mMeshes[i]->mNumVertices);

		InitMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices);
	
		MeshEntry* entry = &m_Entries[i];
		glGenVertexArrays(1, &entry->m_VAO);
		glBindVertexArray(entry->m_VAO);

		// Create the buffers for the vertices attributes
		glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(entry->m_Buffers), entry->m_Buffers);

		// Generate and populate the buffers with vertex attributes and the indices
		glBindBuffer(GL_ARRAY_BUFFER, entry->m_Buffers[POS_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(POSITION_LOCATION);
		glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, entry->m_Buffers[TEXCOORD_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(TEX_COORD_LOCATION);
		glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, entry->m_Buffers[NORMAL_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(NORMAL_LOCATION);
		glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, entry->m_Buffers[BONE_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(BONE_ID_LOCATION);
		
		//glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
		glVertexAttribPointer(BONE_ID_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)0);
		glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
		glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entry->m_Buffers[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	if (!InitMaterials(pScene, Filename)) {
		return false;
	}
	return true;
}

void AssimpMesh::InitMesh(unsigned int MeshIndex,
	const aiMesh* paiMesh,
	std::vector<glm::vec3>& Positions,
	std::vector<glm::vec3>& Normals,
	std::vector<glm::vec2>& TexCoords,
	std::vector<VertexBoneData>& Bones,
	std::vector<unsigned int>& Indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
		Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
		TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
	}

	if (paiMesh->mNumBones > 0) {
		m_animated = true;
		LoadBones(MeshIndex, paiMesh, Bones);
	}
	
	// Populate the index buffer
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}
}

void AssimpMesh::LoadBones(unsigned int MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
{
	//initialize the bone matrices
	for (int bt = 0; bt < MAX_BONES; bt++) {
		BoneTransforms.push_back(glm::mat4(1.0f));
	}

	for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
		unsigned int BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
			// Allocate an index for a new bone
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);
			m_BoneInfo[BoneIndex].BoneOffset = AiToGLMMat4(pMesh->mBones[i]->mOffsetMatrix);
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else {
			BoneIndex = m_BoneMapping[BoneName];
		}

		for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
			unsigned int VertexID = /*m_Entries[MeshIndex].BaseVertex +*/ pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
			Bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}
}

glm::mat4&
AssimpMesh::GetBoneMatrixByName(char* name)
{
	glm::mat4 matrix = glm::mat4(1.0f);
	unsigned int BoneIndex = 0;
	std::string BoneName(name);

	if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
		return matrix;
	}
	else {
		BoneIndex = m_BoneMapping[BoneName];
	}

	// remove the global inverse and the boneoffset from the final transformation
	// to get global transformation
	glm::mat4 globalTransform = glm::inverse(m_GlobalInverseTransform);
	glm::mat4 boneOffsetInv = glm::inverse(m_BoneInfo[BoneIndex].BoneOffset);
	glm::mat4 finalTransform = BoneTransforms.at(BoneIndex);

	return (finalTransform * globalTransform * boneOffsetInv);
}

bool AssimpMesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
	// Extract the directory part from the file name
	std::string::size_type SlashIndex = Filename.find_last_of("/");
	std::string Dir;

	if (SlashIndex == std::string::npos) {
		Dir = ".";
	}
	else if (SlashIndex == 0) {
		Dir = "/";
	}
	else {
		Dir = Filename.substr(0, SlashIndex);
	}

	bool Ret = true;

	// Initialize the materials
	for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		m_Textures[i] = NULL;

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString Path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string p(Path.data);

				if (p.substr(0, 2) == ".\\") {
					p = p.substr(2, p.size() - 2);
				}

				std::string FullPath = Dir + "/" + p;

				m_Textures[i] = new Texture2D();
				if (m_Textures[i]->LoadFromAssetDirect(FullPath.c_str()) == false)
					SAFE_DELETE(m_Textures[i]);
			}
		}
	}

	return Ret;
}

void AssimpMesh::Render()
{
	for (unsigned i = 0; i < m_Entries.size(); i++) {
		const unsigned MaterialIndex = m_Entries[i].MaterialIndex;

		assert(MaterialIndex < m_Textures.size());

		if (m_Textures[MaterialIndex]) {
			m_Textures[MaterialIndex]->Bind(/*GL_TEXTURE0*/);
		}

		MeshEntry* entry = &m_Entries[i];
		glBindVertexArray(entry->m_VAO);
		glDrawElements(GL_TRIANGLES,
			m_Entries[i].NumIndices,
			GL_UNSIGNED_INT,
			(void*)0);
		glBindVertexArray(0);
	}
}

void AssimpMesh::Update(float elapsed)
{
	//...
	AssimpAnimation* animation = &_animationList[_currentAnimationIndex];

	if (animation->done) return;

	_currentAnimationTime += elapsed;
	_currentAnimationTime += animation->start;
	if (_currentAnimationTime > animation->end)
	{
		if (animation->loop) {
			_currentAnimationTime = 0;
		}
		else {
			//... End animation, callback
			animation->done = true;
		}
	}

	BoneTransform(_currentAnimationTime, BoneTransforms);
}

void AssimpMesh::SetAnimation(int index)
{
	//...
	_currentAnimationTime = 0.0f;
	_currentAnimationIndex = index;

	AssimpAnimation* animation = &_animationList[_currentAnimationIndex];
	animation->done = false;

}

void AssimpMesh::AddAnimationByTime(int index, float start,
	float end, bool loop)
{
	//...
	AssimpAnimation animation;
	animation.start = start;
	animation.end = end;
	animation.loop = loop;
	animation.done = false;

	_animationList.insert(std::make_pair(index, animation));
}

void AssimpMesh::AddAnimationByFrame(int index, float start,
	float end, bool loop)
{
	//...
	float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);

	AssimpAnimation animation;
	animation.start = start / TicksPerSecond;
	animation.end = end / TicksPerSecond;
	animation.loop = loop;
	animation.done = false;

	_animationList.insert(std::make_pair(index, animation));
}


void AssimpMesh::BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms)
{
	glm::mat4 Identity = glm::mat4(1.0f);

	float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float TimeInTicks = TimeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)m_pScene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, m_pScene->mRootNode, Identity);

	Transforms.resize(m_NumBones);

	for (unsigned i = 0; i < m_NumBones; i++) {
		Transforms[i] = m_BoneInfo[i].FinalTransformation;
	}
}

void AssimpMesh::ReadNodeHeirarchy(float AnimationTime, aiNode* pNode, const glm::mat4& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = m_pScene->mAnimations[0];

	glm::mat4 NodeTransformation = AiToGLMMat4(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	if (pNodeAnim) {
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		glm::mat4 ScalingM = glm::mat4(1.0f);
		ScalingM = glm::scale(ScalingM, glm::vec3(Scaling.x, Scaling.y, Scaling.z));

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		glm::mat4 RotationM = AiToGLMMat4(RotationQ.GetMatrix());
		
		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		glm::mat4 TranslationM = glm::mat4(1.0f);
		TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
	}

	glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
		unsigned BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (unsigned i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

const aiNodeAnim* AssimpMesh::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (unsigned i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}

void AssimpMesh::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	if (ScalingIndex == -1) return;

	unsigned NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void AssimpMesh::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	if (RotationIndex == -1) return;

	unsigned NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void AssimpMesh::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	if (PositionIndex == -1) return;

	unsigned NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}


unsigned AssimpMesh::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	//assert(0);

	return -1;
}

unsigned AssimpMesh::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	//assert(0);

	return -1;
}

unsigned AssimpMesh::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (unsigned i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	//assert(0);

	return -1;
}


//
// Helper functions
//

static glm::mat4 AiToGLMMat4(aiMatrix4x4& in_mat)
{

	glm::mat4 tmp;

	tmp[0][0] = (GLfloat)in_mat.a1; tmp[0][1] = (GLfloat)in_mat.b1;  tmp[0][2] = (GLfloat)in_mat.c1; tmp[0][3] = (GLfloat)in_mat.d1;
	tmp[1][0] = (GLfloat)in_mat.a2; tmp[1][1] = (GLfloat)in_mat.b2;  tmp[1][2] = (GLfloat)in_mat.c2; tmp[1][3] = (GLfloat)in_mat.d2;
	tmp[2][0] = (GLfloat)in_mat.a3; tmp[2][1] = (GLfloat)in_mat.b3;  tmp[2][2] = (GLfloat)in_mat.c3; tmp[2][3] = (GLfloat)in_mat.d3;
	tmp[3][0] = (GLfloat)in_mat.a4; tmp[3][1] = (GLfloat)in_mat.b4;  tmp[3][2] = (GLfloat)in_mat.c4; tmp[3][3] = (GLfloat)in_mat.d4;
	return tmp;
}

static glm::mat4 AiToGLMMat4(aiMatrix3x3& in_mat)
{
	return glm::mat4(glm::transpose(glm::make_mat3(&in_mat.a1)));
}