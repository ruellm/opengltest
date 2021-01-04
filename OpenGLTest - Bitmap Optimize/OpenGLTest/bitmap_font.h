//
// Bitmap font from Angelcode Bitmap Font Generator
// http://www.angelcode.com/products/bmfont/
// Author: Ruell Magpayo (ruellm@yahoo.com)
// Date: 10-7-2017
//
#pragma once
#include "common.h"
#include "graphics/texture2D.h"
#include "graphics/glshaderfx.h"

namespace ramo {
	namespace graphics {

		struct BmfontInfo
		{
			std::string face;
			int size;
			int bold;
			int italic;
			std::string charset;
			int unicode;
			int stretchH;
			int smooth;
			int aa;
			int padding[4];
			int spacing[2];
			int outline;
		};

		struct BmfontCommon
		{
			int lineHeight;
			int base;
			int scaleW;
			int scaleH;
			int pages;
			int packed;
			int alphaChnl;
			int redChnl;
			int greenChnl;
			int blueChnl;
		};

		struct BmPage
		{
			int id;
			std::string file;
		};

		struct BmChar
		{
			int id;
			int x;
			int y;
			int width;
			int height;
			int xoffset;
			int yoffset;
			int xadvance;
			int page;
			int chnl;
		};

		struct BmKerning
		{
			int first;
			int second;
			int amount;
		};

		class BitmapFont
		{
		public:
			BitmapFont();
			~BitmapFont();

			void LoadFromFile(const char* fname, const char* directory);
			void LoadFromAsset(const char* fname, const char* directory);

			void PrintText(const char* text, int x, int y, GLShaderFx* shader, float scale = 1.0f);
			void PrintTextCenter(const char* text, int cx, int cy, GLShaderFx* shader, float scale = 1.0f);

			// engine definitions
			void SetColor(float r, float g, float b);
			void RestoreColor();
			void SetAlpha(float alpha);

#ifdef RAMO
			static void DrawText(BitmapFont& font, const char* text, int x, int y, float scale = 1.0f);
			static void DrawTextCenter(BitmapFont& font, const char* text, int x, int y, float scale = 1.0f);
#endif

		private:

			struct BmfontInfo				_info;
			struct BmfontCommon				_common;
			std::vector<struct BmPage>		_page;
			int								_chars_count;
			std::map<int, struct BmChar>	_characters;
			int								_kernings_count;
			std::vector<struct BmKerning>	_kerning;
			std::vector<Texture2D*>			_textures;

			void LoadParams_EX(std::vector<std::string>& result, int count);
			std::string TrimQuotes(const std::string in);
			void LoadData(char* path, int size);
			std::string GetData();

			// custom definitions
			glm::vec3		_color;
			float 			_alpha;

			GLuint	_VAO;
			GLuint  _VBO;
			void Initialize();
			void InitalizeTexture(const char* directory);
		};

	}
}