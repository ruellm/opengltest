#include <stdlib.h>
//#include <platform_adapter.h>
#include "obj_model.h"
#include "PathUtility.h"
#include "stb_image.h"
//#include "adapter.h"
#include "tokenizer.h"

//
//TODO: Bitmap font loader uses this technique as well
// why not make a common class?
//
static char* buffer = NULL;
static char* pCurrent = NULL;
static int current_pos = 0;
static int data_size = 0;

OBJModel::OBJModel(void) :
	Model3D()
{
	SetModelType( MODEL_OBJ );
}


OBJModel::~OBJModel(void)
{
	Destroy();
}

void OBJModel::Destroy()
{
	  for(int x=0; x < m_meshList.size(); x++) {
			OBJMesh* meshActual = &m_meshList[x];

			glDeleteBuffers(1, &meshActual->vbo);
			glDeleteBuffers(1, &meshActual->vbo_uv);
			glDeleteBuffers(1, &meshActual->vbo_normals);
			glDeleteVertexArrays(1, &meshActual->vao);
			 
	  }
	  m_meshList.clear();
}


bool OBJModel::LoadFromFile (const char* szFile, const char* szDirectory)
{
	// Store the Path variable
	m_strPathDir = std::string( szDirectory );
	m_strFName = std::string( szFile );

	std::string path = m_strPathDir + "/" + std::string(szFile);

	//if( !ParseFile(path.c_str()) ) return false;
	if( !PortableParseFile(path.c_str()) ) return false;

	//CalcNormals();	//TODO: Calculate Normals

	UploadBuffers( );

	return true;
}

#if 0
// deprecate use Portable version
bool OBJModel::ParseFile(const char* szFile )
{
	//-------------------------------------------------------------------------------------------
	typedef struct OBJMAttributeSet
	{
		std::string material_name;
		std::vector<unsigned int> vertexIndices;
		std::vector<unsigned int> uvIndices;
		std::vector<unsigned int> normalIndices;

	} OBJMGroupSet;
	//-------------------------------------------------------------------------------------------
	
	FILE* fp = NULL;
	std::vector<glm::vec3> out_vertices; 
	std::vector<glm::vec2> out_uvs;
	std::vector<glm::vec3> out_normals;

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices; 
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::vector<std::string> mtllib_list;
	std::string strActiveMatrial;

	// Create attribute mesh object
	OBJMAttributeSet* mesh = NULL;
	std::vector<OBJMAttributeSet> mesh_list;

	//-------------------------------------------------------------------------------------------
	//Loading and parsing file
	fp = fopen(szFile, "r");
	if( fp == NULL ){ return false;	}
	
	while( 1 ) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(fp, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of fp. Quit the loop.

		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(fp, "%f %f\n", &uv.x, &uv.y );
			//uv.y = uv.y; 
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(fp, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){

			std::string vertex1, vertex2, vertex3;

			unsigned int vertexIndex[3]={0}, uvIndex[3] = {0}, normalIndex[3] = {0};

			int matches = fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d\n", 
				&vertexIndex[0], &uvIndex[0], &normalIndex[0], 
				&vertexIndex[1], &uvIndex[1], &normalIndex[1], 
				&vertexIndex[2], &uvIndex[2], &normalIndex[2] );

			if (mesh == NULL) {
				// no material object attribute
				OBJMAttributeSet currmesh;
				currmesh.material_name = "";
				mesh_list.push_back(currmesh);
				mesh = &mesh_list[mesh_list.size() - 1];
			}

			if (matches == 9){
				mesh->vertexIndices.push_back(vertexIndex[0]);
				mesh->vertexIndices.push_back(vertexIndex[1]);
				mesh->vertexIndices.push_back(vertexIndex[2]);
				mesh->uvIndices    .push_back(uvIndex[0]);
				mesh->uvIndices    .push_back(uvIndex[1]);
				mesh->uvIndices    .push_back(uvIndex[2]);
				mesh->normalIndices.push_back(normalIndex[0]);
				mesh->normalIndices.push_back(normalIndex[1]);
				mesh->normalIndices.push_back(normalIndex[2]);	
			} else if( matches == 1) {
				
				matches = fscanf(fp, "%d %d \n", &vertexIndex[1], &vertexIndex[2]); 
				if( matches == 2 ) {
					//only vertex information available: f v1 v2 v3
					mesh->vertexIndices.push_back(vertexIndex[0]);
					mesh->vertexIndices.push_back(vertexIndex[1]);
					mesh->vertexIndices.push_back(vertexIndex[2]);

				}  else if (matches == 0 ){
					// Vertex/normal only f v1//vn1 v2//vn2 v3//vn3 ...
					matches = fscanf(fp, "/%d %d//%d %d//%d\n", 
						&normalIndex[0], 
						&vertexIndex[1],  &normalIndex[1], 
						&vertexIndex[2],  &normalIndex[2]);

					mesh->vertexIndices.push_back(vertexIndex[0]);
					mesh->vertexIndices.push_back(vertexIndex[1]);
					mesh->vertexIndices.push_back(vertexIndex[2]);
					mesh->normalIndices.push_back(normalIndex[0]);
					mesh->normalIndices.push_back(normalIndex[1]);
					mesh->normalIndices.push_back(normalIndex[2]);

				}
			} else if (matches == 2) {
				// Vertex and Texture coords only: f v1/vt1 v2/vt2 v3/vt3 
				 int matches = fscanf(fp, "%d/%d %d/%d \n", 
					&vertexIndex[1], &uvIndex[1], 
					&vertexIndex[2], &uvIndex[2] );

				mesh->vertexIndices.push_back(vertexIndex[0]);
				mesh->vertexIndices.push_back(vertexIndex[1]);
				mesh->vertexIndices.push_back(vertexIndex[2]);
				mesh->uvIndices    .push_back(uvIndex[0]);
				mesh->uvIndices    .push_back(uvIndex[1]);
				mesh->uvIndices    .push_back(uvIndex[2]);
			}

		} else if( strcmp( lineHeader, "mtllib" ) == 0 ){
				// Load material in here
				char szName[100];
				fscanf(fp, "%s", szName);
				std::string mtlname(szName);
				mtllib_list.push_back(szName);

		} else if ( strcmp(lineHeader, "usemtl") == 0)  {
				char szName[100];
				fscanf(fp, "%s", szName);
				std::string mtlname(szName);
				
				OBJMAttributeSet currmesh;
				currmesh.material_name = mtlname;
				mesh_list.push_back(currmesh);
				mesh = &mesh_list[mesh_list.size()-1];

		} else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, fp);
		}
	}
	//-------------------------------------------------------------------------------------------

	//
	// Load the Materials
	//
	std::map < std::string, OBJMMaterial > result;
	LoadMaterials( mtllib_list, result );

	//
	// Calculate Normal vectors
	//
	glm::vec3* pNormals = 0;

	if( temp_normals.size() <= 0 ) {
		pNormals = new glm::vec3[temp_vertices.size()];

		for( int x=0; x < mesh_list.size(); x++ ) {
			OBJMAttributeSet* mesh = &mesh_list[x];

			for( unsigned int i=0; i < mesh->vertexIndices.size(); i += 3 ) {
					unsigned int v0Idx =  mesh->vertexIndices[i]-1;
					unsigned int v1Idx =  mesh->vertexIndices[i+1]-1;
					unsigned int v2Idx =  mesh->vertexIndices[i+2]-1;

					glm::vec3 v0 = temp_vertices[v0Idx];
					glm::vec3 v1 = temp_vertices[v1Idx];
					glm::vec3 v2 = temp_vertices[v2Idx];

					glm::vec3 normal = glm::normalize( glm::cross( v1 - v0, v2 - v0 ) );
				
					pNormals[ v0Idx ] += normal;
					pNormals[ v1Idx ] += normal;
					pNormals[ v2Idx ] += normal;
			 }
		} 

		for(int i = 0; i <  temp_vertices.size(); i++) {
			pNormals[i] = glm::normalize( pNormals[i] ); 
		}
	}
	//
	// Prepare the Data
	//
	for( int x=0; x < mesh_list.size(); x++ ) {

		OBJMAttributeSet* mesh = &mesh_list[x];
		OBJMesh meshActual;

		for( unsigned int i=0; i < mesh->vertexIndices.size(); i++ ) {

            // Get the indices of its attributes
            int vertexIndex = mesh->vertexIndices[i]-1;
			int uvIndex = ( mesh->uvIndices.size() > 0) ? mesh->uvIndices[i] : -1;
			int normalIndex = ( mesh->normalIndices.size() > 0) ? mesh->normalIndices[i] : -1;
                
			meshActual.vertexBuffer.push_back( temp_vertices[ vertexIndex ] );

			if( uvIndex != -1 ) {
				meshActual.texCoordBuffer.push_back( temp_uvs[ uvIndex-1 ] );
			}
           
			
			if(normalIndex != -1 ){ 
				meshActual.normalBuffer.push_back( temp_normals[ normalIndex-1 ] );
			} else {	
				meshActual.normalBuffer.push_back( pNormals[vertexIndex] );
			}

			// Assign materials
			meshActual.material = result[ mesh->material_name ];
		}

		meshActual.hasTexture = ( mesh->uvIndices.size() <= 0 )? false: true;
		meshActual.hasNormal = true;
		//meshActual.hasNormal = ( mesh->normalIndices.size() <= 0 )? false: true;
		m_meshList.push_back(meshActual);

	}

	if( fp ) {	fclose(fp);	}
	SAFE_DELETE(pNormals);


	return true;
}
#endif

void OBJModel::UploadBuffers( )
{
	 for(int x=0; x < m_meshList.size(); x++) {
		OBJMesh* mesh = &m_meshList[x];
		
		glGenVertexArrays(1, &mesh->vao);
		glBindVertexArray(mesh->vao);

		glGenBuffers(1, &mesh->vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

		glBufferData(GL_ARRAY_BUFFER, mesh->vertexBuffer.size() * sizeof(glm::vec3), 
			(void*)&mesh->vertexBuffer[0], GL_STATIC_DRAW);

		 glVertexAttribPointer( 
				0,                           // attribute 0. No particular reason for 0, but must match the layout in the shader. 
				3,                           // size or count of elements 
				GL_FLOAT,           // type 							
				GL_FALSE,           // normalized? 						
				0,                         // stride 					
				(void*)0            // array buffer offset 				
				); 

		if(   mesh->texCoordBuffer.size() > 0 ) {
			glGenBuffers(1, &mesh->vbo_uv);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_uv);

			glBufferData(GL_ARRAY_BUFFER, mesh->texCoordBuffer.size() * sizeof(glm::vec2), 
				(void*)&mesh->texCoordBuffer[0], GL_STATIC_DRAW);

			 glVertexAttribPointer( 
					1,                            
					2,                            
					GL_FLOAT,           							
					GL_FALSE,            						
					0,                   					
					(void*)0            				
					);
		}

		if( mesh->hasNormal ) {
			// Normal Vertex Buffer Object
			glGenBuffers(1, &mesh->vbo_normals);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_normals);

			glBufferData(GL_ARRAY_BUFFER, mesh->normalBuffer.size() * sizeof(glm::vec3), 
				(void*)&mesh->normalBuffer[0], GL_STATIC_DRAW);

			 glVertexAttribPointer( 
					2,                            
					3,                            
					GL_FLOAT,           							
					GL_FALSE,            						
					0,                   					
					(void*)0            				
				); 

		}

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	 }
}

void OBJModel::Draw( )
{
	  for(int x=0; x < m_meshList.size(); x++) {
			OBJMesh* mesh = &m_meshList[x];

			glBindVertexArray(mesh->vao);
		 	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
			glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                  3,                  // size or count of elements
                  GL_FLOAT,           // type
                  GL_FALSE,           // normalized?
                  0,                  // stride
                  (void*)0            // array buffer offset
          );

          if( mesh->hasTexture ) {
              glEnableVertexAttribArray(1);
			  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_uv);
              glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0,(void*)0);
          }
			
			if( mesh->hasNormal ) {
				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_normals);
                glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            }

			GLsizei size = mesh->vertexBuffer.size();

			//Texture Setup
			glActiveTexture(GL_TEXTURE0);
			glBindTexture( GL_TEXTURE_2D, mesh->material.texID );
			glDrawArrays(GL_TRIANGLES, 0, size);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glBindVertexArray(0);

	  }
}

#if 0
void OBJModel::LoadMaterials( const std::vector<std::string>& mtllib_list, 
	std::map<std::string, OBJMMaterial>& result )
{
	OBJMMaterial* pCurrent = NULL;

	for( int i = 0; i < mtllib_list.size(); i++ ) {
		std::string fileterial = mtllib_list[i];
		
		m_mtlFileList.push_back( fileterial );

		FILE* fp = NULL;
		std::string fname = _GetFileName(fileterial.c_str());
		std::string fullpath = 	m_strPathDir + std::string("/") + fname;
		if ( (fp = fopen( fullpath.c_str(), "r" )) == 0 ) continue;

		while ( 1 ) { 
				char lineHeader[128];
				// read the first word of the line
				int res = fscanf(fp, "%s", lineHeader);
				if (res == EOF)
					break; // EOF = End Of fp. Quit the loop.

				if( strcmp( lineHeader, "newmtl" ) == 0 ) {
					// material definition
					char szName[100];
					fscanf(fp, "%s", szName);
					std::string mtlname(szName);

					// new material created
					pCurrent = &result[mtlname];
	
				} else if (strcmp( lineHeader, "Ka" ) == 0) {
					// ambient material
					if( pCurrent == NULL ) continue;
					
					fscanf(fp, "%f %f %f\n", &pCurrent->ambient.x, 
						&pCurrent->ambient.y, &pCurrent->ambient.z);

				} else if (strcmp( lineHeader, "Kd" ) == 0) {
					// diffuse material
					if( pCurrent == NULL ) continue;
					
					fscanf(fp, "%f %f %f\n", &pCurrent->diffuse.x, 
						&pCurrent->diffuse.y, &pCurrent->diffuse.z);

				} else if (strcmp( lineHeader, "Ks" ) == 0) {
					// specular material
					if( pCurrent == NULL ) continue;
					
					fscanf(fp, "%f %f %f\n", &pCurrent->specular.x, 
						&pCurrent->specular.y, &pCurrent->specular.z);

				} else if (strcmp( lineHeader, "map_Kd" ) == 0) {
					// Diffuse Texture map
					if( pCurrent == NULL ) continue;
					
					char szName[100];
					fscanf(fp, "%s", szName);
					std::string mtlname(szName);
					pCurrent->strTextureName = mtlname;


				}
				else{
					// Probably a comment, eat up the rest of the line
					char stupidBuffer[1000];
					fgets(stupidBuffer, 1000, fp);
				}
		}

		if( fp ) fclose(fp);
	}

	//---------------------------------------------
	// !!!NOTE: USE TEXTURE RESOURCE LOADER IN THE ENGINE!!!
    //TODO: centralized this?
	typedef std::map<std::string, OBJMMaterial>::iterator it_type;
	for(it_type it = result.begin(); it != result.end(); it++) {
		OBJMMaterial* material = &it->second;

		//------------------------------------------------------------------------------------------
		// TODO: wrap this in a common texture loader function
		std::string fname = _GetFileName(material->strTextureName.c_str());
		std::string path = m_strPathDir + "/" + fname;

		glGenTextures(1, &material->texID);
		glBindTexture(GL_TEXTURE_2D, material->texID);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			GLuint format = GL_RGB;
			if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);

		}

        glBindTexture(GL_TEXTURE_2D, 0);

		//---------------------------------------------------------------------------------------------------------
	}
}
#endif

void OBJModel::OnSave( const char* szSavePath )
{
#ifdef WIN32
	
	// copy the mesh file
	std::string fullname = GetPathDir() + "\\" 
		+  GetFileName();

	std::string targetmode = std::string(szSavePath) + "\\" +  
		GetFileName();

	BOOL ret = ::CopyFileA( fullname.c_str(),
		targetmode.c_str(),
		FALSE);

	for(int x=0; x < m_meshList.size(); x++) {
			OBJMesh* mesh = &m_meshList[x];

			fullname =  GetPathDir() + "\\" + 
				mesh->material.strTextureName;
			targetmode = std::string(szSavePath) + "\\" +  
					mesh->material.strTextureName;
			
			ret = ::CopyFileA( fullname.c_str(),
				targetmode.c_str(),
				FALSE);

	}

	for(int i=0; i < m_mtlFileList.size(); i++ )
	{
		// copy the mesh file
		std::string fullname = GetPathDir() + "\\" 
			+  m_mtlFileList[i].c_str();

		std::string targetmode = std::string(szSavePath) + "\\" +  
			m_mtlFileList[i].c_str();

		BOOL ret = ::CopyFileA( fullname.c_str(),
			targetmode.c_str(),
			FALSE);
	}
#endif

}

bool OBJModel::PortableParseFile(const char* szFile)
{
	//-------------------------------------------------------------------------------------------
	typedef struct OBJMAttributeSet
	{
		std::string material_name;
		std::vector<unsigned int> vertexIndices;
		std::vector<unsigned int> uvIndices;
		std::vector<unsigned int> normalIndices;

	} OBJMGroupSet;
	//-------------------------------------------------------------------------------------------

	std::vector<glm::vec3> out_vertices;
	std::vector<glm::vec2> out_uvs;
	std::vector<glm::vec3> out_normals;

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::vector<std::string> mtllib_list;
	std::string strActiveMatrial;

	// Create attribute mesh object
	OBJMAttributeSet* mesh = NULL;
	std::vector<OBJMAttributeSet> mesh_list;
	unsigned long size = 0;
	current_pos = 0;
	//-------------------------------------------------------------------------------------------
#if 1
	FILE* fp = NULL;
	fp = fopen(szFile, "r");
	if (fp == NULL) {
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buffer = new char[size + 1];
	memset(buffer, 0, size);
	fread(buffer, size, 1, fp);
	buffer[size] = '\0';
	fclose(fp);
	pCurrent = buffer;
	data_size = size;

#else
	FileData* data = GetPlatform()->GetAssetData(szFile);
    if( data == NULL )
        return false;

	pCurrent = (char*)data->data;
	size = data->data_length;
#endif
	//-------------------------------------------------------------------------------------------
	int count_vertices = 0;
	while (current_pos < size)
	{
		std::string data = GetData();

		if (strcmp(data.c_str(), "v") == 0) {
			std::string v1 = GetData();
			std::string v2 = GetData();
			std::string v3 = GetData();
			glm::vec3 vertex;

			vertex.x = atof(v1.c_str());
			vertex.y = atof(v2.c_str());
			vertex.z = atof(v3.c_str());
			temp_vertices.push_back(vertex);
			count_vertices++;
			if (count_vertices == 431) {
				int bpus = 0;
				bpus++;
			}
		}
		else if (strcmp(data.c_str(), "vt") == 0) {
			std::string v1 = GetData();
			std::string v2= GetData();
			glm::vec2 uv;
			uv.x = atof(v1.c_str());
			uv.y = atof(v2.c_str());
			temp_uvs.push_back(uv);
		}
		else if (strcmp(data.c_str(), "vn") == 0) {
			std::string v1 = GetData();
			std::string v2 = GetData();
			std::string v3 = GetData();

			glm::vec3 normal;
			normal.x = atof(v1.c_str());
			normal.y = atof(v2.c_str());
			normal.z = atof(v3.c_str());
			temp_normals.push_back(normal);
		}
		else if (strcmp(data.c_str(), "f") == 0) {
			// we only support triangle face
			// 3 data only
			std::string v1 = GetData();
			std::string v2 = GetData();
			std::string v3 = GetData();

			Tokens tokens1;
			Tokens tokens2;
			Tokens tokens3;
			Tokenize(v1, tokens1, "/");
			Tokenize(v2, tokens2, "/");
			Tokenize(v3, tokens3, "/");

			if (mesh == NULL) {
				// no material object attribute
				OBJMAttributeSet currmesh;
				currmesh.material_name = "";
				mesh_list.push_back(currmesh);
				mesh = &mesh_list[mesh_list.size() - 1];
			}
			unsigned int vertexIndex[3] = { 0 }, uvIndex[3] = { 0 }, normalIndex[3] = { 0 };

			vertexIndex[0]	= atoi(tokens1.at(0).c_str());
			uvIndex[0]		= atoi(tokens1.at(1).c_str());
			normalIndex[0]	= atoi(tokens1.at(2).c_str());

			vertexIndex[1]	= atoi(tokens2.at(0).c_str());
			uvIndex[1]		= atoi(tokens2.at(1).c_str());
			normalIndex[1]	= atoi(tokens2.at(2).c_str());

			vertexIndex[2]	= atoi(tokens3.at(0).c_str());
			uvIndex[2]		= atoi(tokens3.at(1).c_str());
			normalIndex[2]	= atoi(tokens3.at(2).c_str());

			mesh->vertexIndices.push_back(vertexIndex[0]);
			mesh->vertexIndices.push_back(vertexIndex[1]);
			mesh->vertexIndices.push_back(vertexIndex[2]);
			mesh->uvIndices.push_back(uvIndex[0]);
			mesh->uvIndices.push_back(uvIndex[1]);
			mesh->uvIndices.push_back(uvIndex[2]);
			mesh->normalIndices.push_back(normalIndex[0]);
			mesh->normalIndices.push_back(normalIndex[1]);
			mesh->normalIndices.push_back(normalIndex[2]);

		}
		else if (strcmp(data.c_str(), "mtllib") == 0) {
			// Load material in here
			//char szName[100];
			//fscanf(fp, "%s", szName);
			std::string mtlname = GetData();
			mtllib_list.push_back(mtlname);
		}
		else if (strcmp(data.c_str(), "usemtl") == 0) {

			std::string mtlname = GetData();
			mtllib_list.push_back(mtlname);

			OBJMAttributeSet currmesh;
			currmesh.material_name = mtlname;
			mesh_list.push_back(currmesh);
			mesh = &mesh_list[mesh_list.size() - 1];
		}

		else {
			// this is a comment
			ConsumeLine();
		}
	}
	//
	// Load the Materials
	//
	std::map < std::string, OBJMMaterial > result;
	PortableLoadMaterials(mtllib_list, result);

	//
	// Calculate Normal vectors
	//
	glm::vec3* pNormals = 0;

	if (temp_normals.size() <= 0) {
		pNormals = new glm::vec3[temp_vertices.size()];

		for (int x = 0; x < mesh_list.size(); x++) {
			OBJMAttributeSet* mesh = &mesh_list[x];

			for (unsigned int i = 0; i < mesh->vertexIndices.size(); i += 3) {
				unsigned int v0Idx = mesh->vertexIndices[i] - 1;
				unsigned int v1Idx = mesh->vertexIndices[i + 1] - 1;
				unsigned int v2Idx = mesh->vertexIndices[i + 2] - 1;

				glm::vec3 v0 = temp_vertices[v0Idx];
				glm::vec3 v1 = temp_vertices[v1Idx];
				glm::vec3 v2 = temp_vertices[v2Idx];

				glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

				pNormals[v0Idx] += normal;
				pNormals[v1Idx] += normal;
				pNormals[v2Idx] += normal;
			}
		}

		for (int i = 0; i < temp_vertices.size(); i++) {
			pNormals[i] = glm::normalize(pNormals[i]);
		}
	}
	//
	// Prepare the Data
	//
	for (int x = 0; x < mesh_list.size(); x++) {

		OBJMAttributeSet* mesh = &mesh_list[x];
		OBJMesh meshActual;

		for (unsigned int i = 0; i < mesh->vertexIndices.size(); i++) {

			// Get the indices of its attributes
			int vertexIndex = mesh->vertexIndices[i] - 1;
			int uvIndex = (mesh->uvIndices.size() > 0) ? mesh->uvIndices[i] : -1;
			int normalIndex = (mesh->normalIndices.size() > 0) ? mesh->normalIndices[i] : -1;

			meshActual.vertexBuffer.push_back(temp_vertices[vertexIndex]);

			if (uvIndex != -1) {
				meshActual.texCoordBuffer.push_back(temp_uvs[uvIndex - 1]);
			}


			if (normalIndex != -1) {
				meshActual.normalBuffer.push_back(temp_normals[normalIndex - 1]);
			}
			else {
				meshActual.normalBuffer.push_back(pNormals[vertexIndex]);
			}

			// Assign materials
			meshActual.material = result[mesh->material_name];
		}

		meshActual.hasTexture = (mesh->uvIndices.size() <= 0) ? false : true;
		meshActual.hasNormal = true;
		//meshActual.hasNormal = ( mesh->normalIndices.size() <= 0 )? false: true;
		m_meshList.push_back(meshActual);

	}
	
	SAFE_DELETE(pNormals);
#if 1
	delete[] buffer;
#else
	GetPlatform()->ReleaseAssetData(data);
#endif
	return true;
}
std::string  OBJModel::GetData()
{
	//detect ending
	int index = 0;
	int current = 0;
	char value[100];
	bool end = false;

	while (!end) {
		char c = pCurrent[index++];
		if (c == ' ' || c == '\n' || c == '\r' || c == '\0')
		{
			// handle \r\n combo or CRLF in windows
			if (c == '\n' || c == '\r') {
				if ((current_pos + index + 1 < data_size) &&
					(pCurrent[index] == '\n' || pCurrent[index] == '\r')) {
					++index;
				}
			}


			c = '\0';
			end = true;
		}

		value[current++] = c;
	}
	pCurrent += index;
	current_pos += index;


	return std::string(value);
}

void OBJModel::ConsumeLine()
{
	int index = 0;
	int current = 0;
	bool end = false;

	while (!end) {
		if (current_pos + index >= data_size) {
			goto exit;
		}

		char c = pCurrent[index++];
		if (c == '\n')
		{
			c = '\0';
			end = true;
		}
	}
exit:
	pCurrent += index;
	current_pos += index;
}

void OBJModel::PortableLoadMaterials(const std::vector<std::string>& mtllib_list,
									 std::map<std::string, OBJMMaterial>& result)
{

	OBJMMaterial* pMaterial = NULL;

	for (int i = 0; i < mtllib_list.size(); i++) {
		std::string fileterial = mtllib_list[i];

		m_mtlFileList.push_back(fileterial);
		std::string fname = _GetFileName(fileterial.c_str());
		std::string fullpath = m_strPathDir + std::string("/") + fname;
		unsigned long size = 0;
		current_pos = 0;
		//-------------------------------------------------------------------------------------------
#if 1
		// Repalce this part!!
		FILE* fp = NULL;
		fp = fopen(fullpath.c_str(), "r");
		if (fp == NULL) {
			return;
		}

		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		buffer = new char[size + 1];
		memset(buffer, 0, size);
		fread(buffer, size, 1, fp);
		buffer[size] = '\0';
		fclose(fp);
		pCurrent = buffer;
		data_size = size;
#else
		//-------------------------------------------------------------------------------------------
		FileData* data = GetPlatform()->GetAssetData(fullpath.c_str());
        if( data == NULL )
            continue;

		pCurrent = (char*)data->data;
		size = data->data_length;
#endif
		while (current_pos < size)
		{
			std::string data = GetData();

			if (strcmp(data.c_str(), "newmtl") == 0) {
				// material definition
				std::string mtlname = GetData();

				// new material created
				pMaterial = &result[mtlname];
			}
			else if (strcmp(data.c_str(), "Ka") == 0) {
				if (pMaterial == NULL) continue;
				std::string v1 = GetData();
				std::string v2 = GetData();
				std::string v3 = GetData();

				pMaterial->ambient.x = atof(v1.c_str());
				pMaterial->ambient.y = atof(v2.c_str());
				pMaterial->ambient.z = atof(v3.c_str());
			}
			else if (strcmp(data.c_str(), "Kd") == 0) {
				if (pMaterial == NULL) continue;
				std::string v1 = GetData();
				std::string v2 = GetData();
				std::string v3 = GetData();

				pMaterial->diffuse.x = atof(v1.c_str());
				pMaterial->diffuse.y = atof(v2.c_str());
				pMaterial->diffuse.z = atof(v3.c_str());
			}
			else if (strcmp(data.c_str(), "Ka") == 0) {
				if (pMaterial == NULL) continue;
				std::string v1 = GetData();
				std::string v2 = GetData();
				std::string v3 = GetData();

				pMaterial->specular.x = atof(v1.c_str());
				pMaterial->specular.y = atof(v2.c_str());
				pMaterial->specular.z = atof(v3.c_str());
			}
			else if (strcmp(data.c_str(), "map_Kd") == 0) {
				// Diffuse Texture map
				if (pMaterial == NULL) continue;
				// material definition
				std::string mtlname = GetData();

				pMaterial->strTextureName = mtlname;
			}
			else {
				// probably a comment
				ConsumeLine();
			}
		}
#if 1
		delete[] buffer;
#else
		GetPlatform()->ReleaseAssetData(data);
#endif
		
	}

	//---------------------------------------------
	// Load Texture using SOIL - Opengl library
	// !!!NOTE: USE TEXTURE RESOURCE LOADER IN THE ENGINE!!!
	typedef std::map<std::string, OBJMMaterial>::iterator it_type;
	for (it_type it = result.begin(); it != result.end(); it++) {
		OBJMMaterial* material = &it->second;

		//------------------------------------------------------------------------------------------
		// TODO: wrap this in a common texture loader function
		std::string fname = _GetFileName(material->strTextureName.c_str());
		std::string path = m_strPathDir + "/" + fname;

		glGenTextures(1, &material->texID);
		glBindTexture(GL_TEXTURE_2D, material->texID);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			GLuint format = GL_RGB;
			if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);

		//---------------------------------------------------------------------------------------------------------
	}
}
