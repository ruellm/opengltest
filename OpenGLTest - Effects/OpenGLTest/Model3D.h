#pragma once

#include "common.h"

struct MeshSubset
{
    GLuint vao;
	GLuint vbo;
	GLuint ibo;
	GLuint vbo_uv;
	GLuint texID;

	glm::vec3* m_pVertices;
	unsigned short* m_pIndices;
	int vertCount;
	int indexCount;

	MeshSubset();
	~MeshSubset();
	void ShutDown();

};

typedef enum MODEL_TYPE
{
	MODEL_OBJ,
	MODEL_MD5
} MODEL_TYPE;

class Model3D
{
public:
	Model3D(void);
	virtual ~Model3D(void);

	virtual bool LoadFromFile(const char* szFname, const char* szDirectory);
	virtual void Update( float elapsed );

	virtual int BakeMesh( );
	virtual void Draw( );
	virtual void OnSave( const char* szSavePath ) {}

	inline void SetAnimationIdx( int index ) { m_iCurrentAnim = 0; }
	inline MODEL_TYPE GetModelType( ) { return m_type; }
	inline void SetModelType( MODEL_TYPE type ) { m_type = type; }

	inline std::string GetPathDir() {
		return m_strPathDir;
	}

	inline std::string& GetFileName() {
		return 	m_strFName;
	}

protected:
	std::vector<MeshSubset*> m_subsets;
	int m_nVertexCount;
	int m_nIndexCount;
	bool m_bAnim;

	int m_iCurrentAnim;
	MODEL_TYPE m_type;

	std::string m_strPathDir;
	std::string m_strFName;
	std::string GetPath( const char* szFname );
	
};

