//
// Assimp - Asset Importer adapter (loader and renderer)
// ported from ogldev (ogldev.atspace.co.uk/www/tutorial38/tutorial38.html)
// ruellm@yahoo.com
// Date: Oct 30, 2017
//       Animation handling added Oct 18, 2018
//		 Bone matrix targetting added Oct 25, 2018

#pragma once

#include <map>
#include "common.h"

// Assimp header files
#include "assimp/Importer.hpp"
#include "assimp/scene.h"       // Output data structure
#include "assimp/postprocess.h" // Post processing flags

#include "texture2D.h"

#define MAX_BONES	100

//TODO: move this to common header in engine
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define ZERO_MEM(a) memset(a, 0, sizeof(a))

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

typedef struct _assimpAnimation
{
	float start;
	float end;
	bool loop;
	bool done;

}AssimpAnimation;

class AssimpMesh
{
public:
	AssimpMesh();
	~AssimpMesh();

	bool Load(const std::string& Filename);
	void SetAttributeLocation(
		int position,
		int texture,
		int normal,
		int boneID,
		int boneWeight);

	void Render();
	void BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms);
	bool IsAnimated();

	std::vector<glm::mat4> BoneTransforms;
	glm::mat4& GetBoneMatrixByName(char* name);

	void Update(float elapsed);
	void SetAnimation(int index);
	void AddAnimationByTime(int index, float start, 
		float end, bool loop);
	void AddAnimationByFrame(int index, float start,
		float end, bool loop);


private:

#define NUM_BONES_PER_VEREX 4

	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};

	struct VertexBoneData
	{
		unsigned int IDs[NUM_BONES_PER_VEREX];
		float Weights[NUM_BONES_PER_VEREX];

		VertexBoneData()
		{
			Reset();
		};

		void Reset()
		{
			ZERO_MEM(IDs);
			ZERO_MEM(Weights);
		}

		void AddBoneData(unsigned int BoneID, float Weight);
	};

	#define INVALID_MATERIAL 0xFFFFFFFF
	enum VB_TYPES {
		INDEX_BUFFER,
		POS_VB,
		NORMAL_VB,
		TEXCOORD_VB,
		BONE_VB,
		NUM_VBs
	};

	struct MeshEntry {
		MeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}

		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;

		GLuint m_VAO;
		GLuint m_Buffers[NUM_VBs];
	};

	std::vector<MeshEntry> m_Entries;
	std::vector<Texture2D*> m_Textures;
	std::map<std::string, unsigned int> m_BoneMapping; // maps a bone name to its index
	unsigned int m_NumBones;
	std::vector<BoneInfo> m_BoneInfo;

	// Assimp definitions
	const aiScene* m_pScene;
	Assimp::Importer m_Importer;

	glm::mat4 m_GlobalInverseTransform;
	bool m_animated;

	void CleanUp();
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);
	void InitMesh(unsigned int MeshIndex,
		const aiMesh* paiMesh,
		std::vector<glm::vec3>& Positions,
		std::vector<glm::vec3>& Normals,
		std::vector<glm::vec2>& TexCoords,
		std::vector<VertexBoneData>& Bones,
		std::vector<unsigned int>& Indices);
	bool InitMaterials(const aiScene* pScene, const std::string& Filename);
	void LoadBones(unsigned int MeshIndex, const aiMesh* paiMesh, std::vector<VertexBoneData>& Bones);
	void ReadNodeHeirarchy(float AnimationTime, aiNode* pNode, const glm::mat4& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);

	// animations handling
	float _currentAnimationTime;
	int _currentAnimationIndex;
	std::map<int, AssimpAnimation> _animationList;
};

