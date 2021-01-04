#include "Model3D.h"
#include "PathUtility.h"

MeshSubset::MeshSubset() : 
	vbo(0), ibo(0), texID(0),
	m_pVertices(NULL), m_pIndices(NULL),
	vbo_uv(0), vao(0), 
	vertCount(0), indexCount(0) {
	/*..*/
}

MeshSubset::~MeshSubset(){ 
	ShutDown(); 
}

void MeshSubset::ShutDown() {

	SAFE_DELETE_ARRAY(m_pVertices);
	SAFE_DELETE_ARRAY(m_pIndices);

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	//glDeleteTextures(1, & texID);		  //--> Managed by Texture Loader
	glDeleteVertexArrays(1, &vao);
}

////////////////////////////////////////////////////////////////////////////////////
// Mesh Subset
///////////////////////////////////////////////////////////////////////////////////

Model3D::Model3D(void) 
	: m_nVertexCount(0), m_nIndexCount(0),
	m_bAnim(false), m_iCurrentAnim(0)
{
	//...
}


Model3D::~Model3D(void)
{
	for(int i =0; i < m_subsets.size(); i++) {
		m_subsets[i]->ShutDown();
		SAFE_DELETE(m_subsets[i]);
	}
}

bool Model3D::LoadFromFile(const char* szFname, const char* szDirectory)
{
	return true;
}

void Model3D::Update( float elapsed )
{
	//...
}	

int Model3D::BakeMesh( )
{
	return 0;
}

void Model3D::Draw( )
{
	//...
}

std::string Model3D::GetPath( const char* szFname )
{
	return (m_strPathDir + "/" + _GetFileName( szFname ));
}
