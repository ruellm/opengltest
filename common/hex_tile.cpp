//
// Created by magpayor on 10/17/2017.
//

#include "hex_tile.h"
#include "utility.h"
#include "glshaderfx.h"

#if 0   //TODO: create a switch Macro to toggle Desktop and Android Engine
#include "engine/engine.h"
#include "game_data.h"
#include "resource.h"
#endif

GLuint      HexTile::_VBO = 0;
GLuint		HexTile::_VAO = 0;
Texture2D*   HexTile::_texture;
int         HexTile::_size = 0;

// shader attribute
static int POSITION_ATTRIBUTE = 0;
static int UV_ATTRIBUTE = 1;

HexTile::HexTile() : _object(0)
{}

HexTile::~HexTile()
{
    Cleanup();
}

void HexTile::Cleanup() {
    if( _VBO ) {
        glDeleteBuffers(1, &_VBO);
        _VBO = 0;
    }
}

void HexTile::Load(int size, int position_attrib_id,
	int texture_attrib_id)
{
    _size = size;

    CreateRectangle(_size, position_attrib_id, texture_attrib_id,
		&_VBO
#ifndef OPENGLES2
		, &_VAO
#endif
	);

#if OPENGLES_RAMO
	_texture = ResourceManager::GetInstance()->GetTexture(HEX_TILE_IMG);
#endif
}

void HexTile::Load(int size, 
	int position_attrib_id,
	int texture_attrib_id,
	Texture2D* texture)
{
	_size = size;

	CreateRectangle(_size, position_attrib_id, texture_attrib_id,
		&_VBO
#ifndef OPENGLES2
		, &_VAO
#endif
	);

	_texture = texture;
}

void HexTile::Update(float elapsed)
{
    SceneObject::Update(elapsed);
}

void
HexTile::SetAttribute(int position, int uvattributes)
{
    POSITION_ATTRIBUTE = position;
    UV_ATTRIBUTE = uvattributes;
}

void HexTile::Draw()
{
    _texture->Bind();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef OPENGLES2
	glBindVertexArray(_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
#else

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glVertexAttribPointer(POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(POSITION_ATTRIBUTE);

    glVertexAttribPointer(UV_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(UV_ATTRIBUTE);
    glDrawArrays(GL_TRIANGLES, 0, 6);

#endif
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

}

void HexTile::GenerateBoundingBox()
{
    // since tile is created at 0,0,0
    // position is not needed for bounding box
    m_min.x = -(_size/1);
    m_min.y = 0;
    m_min.z = -(_size/1);

    m_max.x = (_size/1);
    m_max.y = 0;
    m_max.z = (_size/1);
}

void HexTile::SetCharacter(PickableObject* c)
{
    _object = c;
}

PickableObject* HexTile::GetCharacter()
{
    return _object;
}