//
// Created by magpayor on 9/29/2017.
//

#ifndef ANDROID_TEXTURE2D_H
#define ANDROID_TEXTURE2D_H

#include "common.h"

class Texture2D {
public:
    // Constructor (sets default texture modes)
    Texture2D();
    ~Texture2D();

    // Holds the ID of the texture object, used for all texture operations to reference to this particlar texture
    GLuint _id;

    // Texture image dimensions
    GLuint _width, _height;         // Width and height of loaded image in pixels

    // Texture Format
    GLuint _format;        // Format of texture object

    // Texture configuration
    GLuint _wrap_s;                 // Wrapping mode on S axis
    GLuint _wrap_t;                 // Wrapping mode on T axis
    GLuint _filter_min;              // Filtering mode if texture pixels < screen pixels
    GLuint _filter_max;              // Filtering mode if texture pixels > screen pixels

    // Generates texture from image data
    void Generate(GLuint width, GLuint height, unsigned char* data);

    // Binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind() const;

    void LoadFromAssetDirect(const char* szPath);
    //
    //TODO: load from file directly support
    // void LoadFromFile(const char* szPath);
};


#endif //ANDROID_TEXTURE2D_H
