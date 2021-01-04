//
// Bitmap font from Angelcode Bitmap Font Generator
// http://www.angelcode.com/products/bmfont/
// Author: Ruell Magpayo (ruellm@yahoo.com)
// Date: 10-7-2017
#include "bitmap_font.h"
#include "tokenizer.h"
#include "common.h"
#include "stb_image.h"
#include "GLShaderFx.h"

GLShaderFx shader;
static char* buffer = NULL;
static char* pCurrent = NULL;
static int current_pos = 0;

BitmapFont::BitmapFont()
{
}


BitmapFont::~BitmapFont()
{
	for (int t = 0; t < _textures.size(); t++) {
		glDeleteTextures(1, &_textures.at(t));
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

void BitmapFont::LoadParams(FILE* fp, std::vector<std::string>& result, int count)
{
	int i = 0;
	char params[100];
	while (i++ < count) {
		fscanf(fp, "%s", params);
		std::string strParams(params);
		result.push_back(strParams);
	}
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

	//------------------------------------------------------------
	// Temporary!! shader loading
	shader.Init();
	shader.LoadVertexShaderFromFile("./shaders/sprite2D_vshader");
	shader.LoadFragmentShaderFromFile("./shaders/sprite2D_fshader");
	shader.Build();
	//------------------------------------------------------------

	// Replace this with engine's texture wrapper class
	// Build textures
	for (int p = 0; p < _common.pages; p++) {
		struct BmPage page = _page[p];

		std::string texturePath = std::string(directory) +
			"/" + page.file;

		unsigned int texture1;
		glGenTextures(1, &texture1);
		glBindTexture(GL_TEXTURE_2D, texture1);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);
		_textures.push_back(texture1);
	}

	if (fp)
	{
		fclose(fp);
	}
}


void BitmapFont::LoadData(char* buffer, int size)
{
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

GLenum error;
void BitmapFont::PrintText(const char* text, int x, int y, float scale)
{
	//Note: y parameter must specify the bottom left
	int length = strlen(text);
	int current_x = x;
	int current_y = y-(_common.base *scale);

	for (int i = 0; i < length; i++) {
		int character = text[i];

		struct BmChar charInfo;
		std::map<int, struct BmChar>::iterator it 
				=  _characters.find(character);
		if (it == _characters.end()) {
			continue;
		}

		charInfo = it->second;
		current_y = y + (charInfo.yoffset * scale);

		float top_left_X = current_x;
		float top_left_Y = current_y;

		float top_right_X = current_x + (charInfo.width * scale);
		float top_right_Y = top_left_Y;

		float bottom_right_X = top_right_X;
		float bottom_right_Y = current_y + (charInfo.height * scale);

		float bottom_left_X = top_left_X;
		float bottom_left_Y = bottom_right_Y;

		float u_top_left = charInfo.x / (float)_common.scaleW;
		float v_top_left = charInfo.y / (float)_common.scaleH;		// invert this in actual

		float u_bottom_right = (charInfo.x + charInfo.width) / (float)_common.scaleW;
		float v_bottom_right = (charInfo.y + charInfo.height) / (float)_common.scaleH;

		float vertices[] = {
			// first triangle									texture coordinates
			top_right_X,		top_right_Y,		0.0f,		u_bottom_right,	v_top_left,			// top right
			bottom_right_X,		bottom_right_Y,		0.0f,		u_bottom_right,	v_bottom_right,		// bottom right
			top_left_X,			top_left_Y,			0.0f,		u_top_left,		v_top_left,			// top left 
			// second triangle
			bottom_right_X,		bottom_right_Y,		0.0f,		u_bottom_right,	v_bottom_right,		// bottom right
			bottom_left_X,		bottom_left_Y,		0.0f,		u_top_left,		v_bottom_right,		// bottom left
			top_left_X,			top_left_Y,			0.0f,		u_top_left,		v_top_left			// top left
		};

		//-------------------------------------------------------------------
		// TEMP: for testing only
		// shader setup
		glm::mat4 projection = glm::ortho(0.0f, (float)800,
			(float)600, 0.0f, -1.0f, 1.0f);

		shader.Use();
		int projmtx = shader.GetUniformLocation("projection");
		int modelidx = shader.GetUniformLocation("model");
		int textureID = shader.GetUniformLocation("image");

		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(scale, scale, 1.0f));

		shader.SetUniformMatrix(projmtx, projection);
		shader.SetUniformMatrix(modelidx, model);
		//-------------------------------------------------------------------

		GLuint texture = _textures.at(charInfo.page);
		glUniform1i(textureID, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// Enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		current_x += (charInfo.xadvance * scale);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &VBO);
	}
}