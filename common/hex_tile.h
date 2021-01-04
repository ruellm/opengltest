//
// Created by magpayor on 10/17/2017.
//

#ifndef RAMO_HEX_TILE_H
#define RAMO_HEX_TILE_H

#include "scene_object.h"
#include "texture2D.h"
#include "scene_object.h"
//#include "game_character.h"

class HexTile : public PickableObject
{
public:
    HexTile();
    ~HexTile();

    static void Load(int size, int position_attrib_id,
		int texture_attrib_id);

	static void Load(int size, int position_attrib_id,
		int texture_attrib_id, Texture2D* texture);

    static void Cleanup();

    virtual void Update(float elapsed);
    virtual void Draw();
    virtual void GenerateBoundingBox();

    void SetCharacter(PickableObject* c);
    PickableObject* GetCharacter();

    static void SetAttribute(int position, int uvattributes);

private:
    static GLuint      _VBO;
	static GLuint		_VAO;
    static Texture2D*   _texture;
    static int         _size;
    PickableObject*  _object;
};


#endif //RAMO_HEX_TILE_H
