#pragma once

#include "common.h"
#include "Model3D.h"

typedef struct OBJMMaterial
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float transparency;

	std::string strTextureName;
	std::string strMtlFile;
	GLuint texID;

} OBJMMaterial;

typedef struct OBJMesh
{	 
	 OBJMMaterial material;
	 bool hasTexture;
	 bool hasNormal;
	 std::vector<glm::vec3> vertexBuffer;
	 std::vector<glm::vec2> texCoordBuffer;
	 std::vector<glm::vec3> normalBuffer;
	 GLuint vbo;
	 GLuint vbo_uv;
	 GLuint vbo_normals;
	 GLuint vao;

} OBJMesh;

class OBJModel : public Model3D
{
public:
	OBJModel(void);
	~OBJModel(void);

	bool LoadFromFile (const char* szFile, const char* szDirectory );
	bool LoadFromFileEX(const char* szFile, const char* szDirectory);
	void Destroy();
	void Draw( );
	virtual void OnSave( const char* szSavePath );

	inline OBJMesh* GetMesh(int i ) {
		return &m_meshList[i];
	}
	inline int GetMeshCount() {
		return m_meshList.size();
	}

private:
	std::vector<OBJMMaterial> m_textureList;
	std::vector<OBJMesh> m_meshList;

	// only use for file saving, keep track of the material filenames
	std::vector<std::string> m_mtlFileList;		

#if 1
	void LoadMaterials( const std::vector<std::string>& mtllib_list, 
		std::map<std::string, OBJMMaterial>& result );

	bool ParseFile(const char* szFile);
#endif

	void UploadBuffers( );

	bool PortableParseFile(const char* szFile);
	std::string  GetData();
	void ConsumeLine();
	void PortableLoadMaterials(const std::vector<std::string>& mtllib_list,
							   std::map<std::string, OBJMMaterial>& result);
};

