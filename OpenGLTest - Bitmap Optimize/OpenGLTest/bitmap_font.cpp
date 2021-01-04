//
// Bitmap font from Angelcode Bitmap Font Generator
// http://www.angelcode.com/products/bmfont/
// Author: Ruell Magpayo (ruellm@yahoo.com)
// Date: 10-7-2017
// update on 11/14/2018 for Optimization, create VBO/VAO and triangle once
// and pass texture coordinate as uniform, texture position as attribute

#include <stdlib.h>
#include "utility/tokenizer.h"
#include "common.h"
#include "stb_image.h"
#include "graphics/GLShaderFx.h"
#include "engine/engine.h"
#include "adapter.h"

#ifdef RAMO
#include "graphics/bitmap_font.h"
#else
// enable this when doing unit test
#include "bitmap_font.h"
#endif


using namespace ramo::graphics;

//
//TODO: OBJ loader uses this technique as well
// why not make a common class?
//
static char* buffer = NULL;
static char* pCurrent = NULL;
static int current_pos = 0;

BitmapFont::BitmapFont()
	: _alpha(1.0f)
	, _VAO(0)
	, _VBO(0)
{
	SetColor(1, 1, 1);
}


BitmapFont::~BitmapFont()
{
	for (int t = 0; t < _textures.size(); t++) {
		SAFE_DELETE(_textures.at(t));
	}

	if (_VAO) {
		glDeleteVertexArrays(1, &_VAO);
	}
	if (_VBO) {
		glDeleteBuffers(1, &_VBO);
	}
}


std::string BitmapFont::TrimQuotes(std::string in)
{
	size_t pos;
	while (std::string::npos != (pos = in.find("\""))) {
		in = in.erase(pos, 1);
	}
	return in;
}

void BitmapFont::LoadParams_EX(std::vector<std::string>& result, int count)
{
	int i = 0;
	while (i < count) {
		std::string strParams = GetData();
		if (strParams.size() > 0)
		{
			result.push_back(strParams);
			i++;
		}
	}
}

void BitmapFont::LoadFromFile(const char* fname, 
	const char* directory)
{
	FILE* fp = NULL;
	std::string strPath = std::string(directory) + 
		"/" + std::string(fname);

	fp = fopen(strPath.c_str(),"r");
	if (fp ==  NULL) {
		return;
	}

    fseek(fp, 0, SEEK_END);
    unsigned long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = new char[size + 1];
    memset(buffer, 0, size);
    fread(buffer, size, 1, fp);
    buffer[size] = '\0';

    LoadData(buffer, size + 1);
    delete[] buffer;

	InitalizeTexture(directory);

	if (fp)
	{
		fclose(fp);
	}
}

void BitmapFont::InitalizeTexture(const char* directory)
{
	for (int p = 0; p < _common.pages; p++) {
		struct BmPage page = _page[p];

		std::string texturePath = std::string(directory) +
			"/" + page.file;

		Texture2D* texture = new Texture2D();
		texture->LoadFromAssetDirect(texturePath.c_str());
		_textures.push_back(texture);
	}
}

#ifdef RAMO
void BitmapFont::LoadFromAsset(const char* fname, const char* directory)
{
    std::string strPath = std::string(directory) +
                          "/" + std::string(fname);

    FileData* data = GetPlatform()->GetAssetData(strPath.c_str());
    LoadData( (char*)data->data, data->data_length );
    GetPlatform()->ReleaseAssetData(data);

	InitalizeTexture(directory);
}
#endif

void BitmapFont::LoadData(char* buffer, int size)
{
	// prepare triangles
	Initialize();

	pCurrent = buffer;
	current_pos = 0;

	while (current_pos < size)
	{
		std::string data = GetData();

		if (strcmp(data.c_str(), "info") == 0) {
			std::vector<std::string> temp;
			LoadParams_EX(temp, 12);
			for (int i = 0; i < temp.size(); i++) {
				Tokens tokens;
				Tokenize(temp.at(i), tokens, "=");
				std::string attrib = tokens.at(0);
				std::string value = tokens.at(1);
				if (attrib == "face") {
					_info.face = TrimQuotes(value);
				}
				else if (attrib == "size") {
					_info.size = atoi(value.c_str());
				}
				else if (attrib == "bold") {
					_info.bold = atoi(value.c_str());
				}
				else if (attrib == "italic") {
					_info.italic = atoi(value.c_str());
				}
				else if (attrib == "charset") {
					_info.charset = TrimQuotes(value);
				}
				else if (attrib == "unicode") {
					_info.unicode = atoi(value.c_str());
				}
				else if (attrib == "stretchH") {
					_info.stretchH = atoi(value.c_str());
				}
				else if (attrib == "smooth") {
					_info.smooth = atoi(value.c_str());
				}
				else if (attrib == "aa") {
					_info.aa = atoi(value.c_str());
				}
				else if (attrib == "padding") {
					Tokens tokens_2;
					Tokenize(value, tokens_2, ",");
					for (int v = 0;v < 4;v++) {
						_info.padding[v] = atoi(tokens_2.at(v).c_str());
					}
				}
				else if (attrib == "spacing") {
					Tokens tokens_2;
					Tokenize(value, tokens_2, ",");
					for (int v = 0;v < 2;v++) {
						_info.spacing[v] = atoi(tokens_2.at(v).c_str());
					}
				}
				else if (attrib == "outline") {
					_info.outline = atoi(value.c_str());
				}
			} //end of for
		} // end of if
		else if (strcmp(data.c_str(), "common") == 0) {
			std::vector<std::string> temp;
			LoadParams_EX(temp, 10);
			for (int i = 0; i < temp.size(); i++) {
				Tokens tokens;
				Tokenize(temp.at(i), tokens, "=");
				std::string attrib = tokens.at(0);
				std::string value = tokens.at(1);
				if (attrib == "lineHeight") {
					_common.lineHeight = atoi(value.c_str());
				}
				else if (attrib == "base") {
					_common.base = atoi(value.c_str());
				}
				else if (attrib == "scaleW") {
					_common.scaleW = atoi(value.c_str());
				}
				else if (attrib == "scaleH") {
					_common.scaleH = atoi(value.c_str());
				}
				else if (attrib == "pages") {
					_common.pages = atoi(value.c_str());
				}
				else if (attrib == "packed") {
					_common.packed = atoi(value.c_str());
				}
				else if (attrib == "alphaChnl") {
					_common.alphaChnl = atoi(value.c_str());
				}
				else if (attrib == "redChnl") {
					_common.redChnl = atoi(value.c_str());
				}
				else if (attrib == "greenChnl") {
					_common.greenChnl = atoi(value.c_str());
				}
				else if (attrib == "blueChnl") {
					_common.blueChnl = atoi(value.c_str());
				}
			}//end for
		} // end else if
		else if (strcmp(data.c_str(), "page") == 0) {
			std::vector<std::string> temp;
			LoadParams_EX(temp, 2);
			struct BmPage page;

			for (int i = 0; i < temp.size(); i++) {
				Tokens tokens;
				Tokenize(temp.at(i), tokens, "=");
				std::string attrib = tokens.at(0);
				std::string value = tokens.at(1);
				if (attrib == "id") {
					page.id = atoi(value.c_str());
				}
				else if (attrib == "file") {
					page.file = TrimQuotes(value);
				}
			}// end of for
			_page.push_back(page);
		}// end of else if
		else if (strcmp(data.c_str(), "chars") == 0) {
			std::string count = GetData();

			Tokens tokens;
			Tokenize(count, tokens, "=");
			std::string attrib = tokens.at(0);
			std::string value = tokens.at(1);

			_chars_count = atoi(value.c_str());
		}
		else if (strcmp(data.c_str(), "char") == 0) {
			std::vector<std::string> temp;
			LoadParams_EX(temp, 10);
			struct BmChar character;

			for (int i = 0; i < temp.size(); i++) {
				Tokens tokens;
				Tokenize(temp.at(i), tokens, "=");
				std::string attrib = tokens.at(0);
				std::string value = tokens.at(1);
				if (attrib == "id") {
					character.id = atoi(value.c_str());
				}
				else if (attrib == "x") {
					character.x = atoi(value.c_str());
				}
				else if (attrib == "y") {
					character.y = atoi(value.c_str());
				}
				else if (attrib == "width") {
					character.width = atoi(value.c_str());
				}
				else if (attrib == "height") {
					character.height = atoi(value.c_str());
				}
				else if (attrib == "xoffset") {
					character.xoffset = atoi(value.c_str());
				}
				else if (attrib == "yoffset") {
					character.yoffset = atoi(value.c_str());
				}
				else if (attrib == "xadvance") {
					character.xadvance = atoi(value.c_str());
				}
				else if (attrib == "page") {
					character.page = atoi(value.c_str());
				}
				else if (attrib == "chnl") {
					character.chnl = atoi(value.c_str());
				}
			}
			// insert to map
			_characters.insert(
				std::make_pair(character.id, character));
		}
		else if (strcmp(data.c_str(), "kernings") == 0) {
			//fscanf(fp, "%d", &_kernings_count);
			std::string count = GetData();

			Tokens tokens;
			Tokenize(count, tokens, "=");
			std::string attrib = tokens.at(0);
			std::string value = tokens.at(1);

			_kernings_count = atoi(value.c_str());
		}
		else if (strcmp(data.c_str(), "kerning") == 0) {
			std::vector<std::string> temp;
			LoadParams_EX(temp, 3);
			struct BmKerning kerning;

			for (int i = 0; i < temp.size(); i++) {
				Tokens tokens;
				Tokenize(temp.at(i), tokens, "=");
				std::string attrib = tokens.at(0);
				std::string value = tokens.at(1);
				if (attrib == "first") {
					kerning.first = atoi(value.c_str());
				}
				else if (attrib == "second") {
					kerning.second = atoi(value.c_str());
				}
				else if (attrib == "amount") {
					kerning.amount = atoi(value.c_str());
				}
			}
			_kerning.push_back(kerning);
		}
	}
}

std::string  BitmapFont::GetData()
{
	//detect ending
	int index = 0;
	int current = 0;
	char value[100];
	bool end = false;
	while (!end) {
		char c = pCurrent[index++];
		if (c == ' ' || c == '\n' || c == '\r' || c=='\0')
		{
			c = '\0';
			end = true;
		}

		value[current++] = c;
	}
	pCurrent += index;
	current_pos += index;

	return std::string(value);
}

void BitmapFont::PrintText(const char* text, int x, int y, GLShaderFx* shader, float scale)
{
	//Note: y parameter must specify the bottom left
	int length = strlen(text);
	int current_x = x;
	int topY = y-(_common.base * scale);

	for (int i = 0; i < length; i++) {
		int character = text[i];

		struct BmChar charInfo;
		std::map<int, struct BmChar>::iterator it 
				=  _characters.find(character);
		if (it == _characters.end()) {
			continue;
		}

		charInfo = it->second;
		int current_y = topY + (charInfo.yoffset * scale);

		float u_top_left = charInfo.x / (float)_common.scaleW;
		float v_top_left = charInfo.y / (float)_common.scaleH;		// invert this in actual

		float u_bottom_right = (charInfo.x + charInfo.width) / (float)_common.scaleW;
		float v_bottom_right = (charInfo.y + charInfo.height) / (float)_common.scaleH;

		int textureID = shader->GetUniformLocation("image");
        Texture2D* texture = _textures.at(charInfo.page);
        glUniform1i(textureID, 0);
        glActiveTexture(GL_TEXTURE0);

		texture->Bind();

		int alphaid = shader->GetUniformLocation("alpha");
		shader->SetUniformFloat(alphaid, _alpha);

		int color = shader->GetUniformLocation("color");
		shader->SetUniformVector3(color, _color);

		int topleft_u = shader->GetUniformLocation("u_top_left");
		shader->SetUniformFloat(topleft_u, u_top_left);

		int topleft_v = shader->GetUniformLocation("v_top_left");
		shader->SetUniformFloat(topleft_v, v_top_left);

		int bottom_right_u = shader->GetUniformLocation("u_bottom_right");
		shader->SetUniformFloat(bottom_right_u, u_bottom_right);

		int bottom_right_v = shader->GetUniformLocation("v_bottom_right");
		shader->SetUniformFloat(bottom_right_v, v_bottom_right);

		// Prepare transformations
		glm::mat4 model = glm::mat4(1.0f);	//initialize to identity

		// First translate (transformations are: scale happens first,
		// then rotation and then finall translation happens; reversed order)
		model = glm::translate(model, glm::vec3(current_x, current_y, 0.0f));

		//Resize to current scale
		model = glm::scale(model, glm::vec3(charInfo.width * scale, 
			charInfo.height * scale, 1.0f));

		int modelidx = shader->GetUniformLocation("model");
		shader->SetUniformMatrix(modelidx, model);

		// Enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef OPENGLES2
		glBindVertexArray(_VAO);
#else
		glBindBuffer(GL_ARRAY_BUFFER, _VBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

#endif
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifndef OPENGLES2
		glBindVertexArray(0);
#endif
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		current_x += (charInfo.xadvance * scale);
	}
}

void BitmapFont::PrintTextCenter(const char* text, int cx, int cy, GLShaderFx* shader, float scale)
{
	int x = 0;
	int y = 0;
	int total_width = 0;
	int length = strlen(text);

	for (int i = 0; i < length; i++) {
		int character = text[i];

		struct BmChar charInfo;
		std::map<int, struct BmChar>::iterator it = _characters.find(character);
		if (it == _characters.end()) {
			continue;
		}

		charInfo = it->second;
		total_width += (charInfo.width * scale);
	}
	x = cx - (total_width/2.0f);
	y = cy;

	BitmapFont::PrintText(text, x, y, shader, scale);
}

void BitmapFont::SetColor(float r, float g, float b)
{
	_color = glm::vec3(r, g, b);
}

void BitmapFont::RestoreColor()
{
    _color = glm::vec3(1, 1, 1);
}

void BitmapFont::SetAlpha(float alpha)
{
	_alpha = alpha;
}


#ifdef RAMO
void BitmapFont::DrawText(BitmapFont& font, const char* text, int x, int y, float scale)
{
	GLShaderFx* shader = Engine::GetGraphics()->GetShader(SHADERS_FONT);
	shader->Use();
	glm::mat4 projection = Engine::GetGraphics()->GetOrthoProjection();
	int projmtx = shader->GetUniformLocation("projection");
	shader->SetUniformMatrix(projmtx, projection);

	int modelidx = shader->GetUniformLocation("model");
	glm::mat4 model = glm::mat4(1.0f);
	shader->SetUniformMatrix(modelidx, model);

	font.PrintText(text, x, y, shader, scale);
}

void BitmapFont::DrawTextCenter(BitmapFont& font, const char* text, int x, int y, float scale)
{
	GLShaderFx* shader = Engine::GetGraphics()->GetShader(SHADERS_FONT);
	shader->Use();
	glm::mat4 projection = Engine::GetGraphics()->GetOrthoProjection();
	int projmtx = shader->GetUniformLocation("projection");
	shader->SetUniformMatrix(projmtx, projection);

	int modelidx = shader->GetUniformLocation("model");
	glm::mat4 model = glm::mat4(1.0f);
	shader->SetUniformMatrix(modelidx, model);

	font.PrintTextCenter(text, x, y, shader, scale);
}
#endif

void BitmapFont::Initialize()
{
	// texture position
	// 0 top right
	// 1 bottom right
	// 2 top left
	// 3 bottom left

	//coordinates in ortho/screenspace
	GLfloat vertices[] = {
		// Pos		// texture position
		0.0f, 1.0f,	 3,			// bottom left
		1.0f, 0.0f,	 0,			// top right
		0.0f, 0.0f,	 2,			// top left

		0.0f, 1.0f,	 3,         // bottom left
		1.0f, 1.0f,	 1,         // bottom right
		1.0f, 0.0f,	 0          // top right
	};

#ifndef OPENGLES2
	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(this->_VAO);
#endif

	glGenBuffers(1, &_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifndef OPENGLES2
	glBindVertexArray(0);
#endif
}