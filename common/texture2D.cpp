//
// Created by magpayor on 9/29/2017.
//

#include "stb_image.h"
#include "texture2D.h"


// Constructor (sets default texture modes)
Texture2D::Texture2D()
    : _width(0), _height(0), _format(GL_RGB), _wrap_s(GL_REPEAT),
      _wrap_t(GL_REPEAT), _filter_min(GL_LINEAR), _filter_max(GL_LINEAR)
{
   //...
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &_id);
}

void Texture2D::Generate(GLuint width, GLuint height, unsigned char* data)
{
    this->_width = width;
    this->_height = height;

    // Create Texture
	glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexImage2D(GL_TEXTURE_2D, 0, _format, width, height, 0,_format, GL_UNSIGNED_BYTE, data);

    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrap_t);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filter_min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filter_max);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

//TODO: must be loaded from resource manager
bool Texture2D::LoadFromAssetDirect(const char* szPath)
{
#if 0
    FileData* fileData = GetPlatform()->GetAssetData(szPath);

    int channels = 0;
    unsigned char *data = 0;
    data = stbi_load_from_memory((unsigned char*)fileData->data,
                                 fileData->data_length,
                                 (int*)&_width, (int*)&_height,&channels,0 );
#endif
	int width, height, nrChannels;
	unsigned char *data = stbi_load(szPath, &width, &height, &nrChannels, 0);

	if( data == NULL ) {
       // Log( "GAME CORE", "[ ERROR]: Unable to load image %s", szPath);
		return false;
    }

    if(nrChannels == 3){
        _format = GL_RGB;
    }else if(nrChannels == 4) {
        _format = GL_RGBA;
    }
	_width = width;
	_height = height;

    Generate( _width, _height, data);
    stbi_image_free(data);

	return true;
}

void Texture2D::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, _id);
}