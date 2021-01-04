#include "RavineLevel.h"
#include "common.h"

RavineLevel::RavineLevel()
{
}


RavineLevel::~RavineLevel()
{
}

void RavineLevel::Initialize()
{
	_modelShader.Init();
	_modelShader.LoadVertexShaderFromFile("./shaders/model_vshader.txt");
	_modelShader.LoadFragmentShaderFromFile("./shaders/model_fshader.txt");
	_modelShader.Build();

	_shaderOrtho.Init();
	_shaderOrtho.LoadVertexShaderFromFile("./shaders/basic_vshader_ortho.txt");
	_shaderOrtho.LoadFragmentShaderFromFile("./shaders/basic_fshader_ortho.txt");
	_shaderOrtho.Build();

	_blurShader.Init();
	_blurShader.LoadVertexShaderFromFile("./shaders/basic_vshader_ortho.txt");
	_blurShader.LoadFragmentShaderFromFile("./shaders/blur_fs.txt");
	_blurShader.Build();

	_bloomFinal.Init();
	_bloomFinal.LoadVertexShaderFromFile("./shaders/basic_vshader_ortho.txt");
	_bloomFinal.LoadFragmentShaderFromFile("./shaders/bloom_final_fs.txt");
	_bloomFinal.Build();

	_brightFilter.Init();
	_brightFilter.LoadVertexShaderFromFile("./shaders/model_vshader.txt");
	_brightFilter.LoadFragmentShaderFromFile("./shaders/brightness_filter_fshader.txt");
	_brightFilter.Build();

	_color1.Initialize(SCR_WIDTH, SCR_HEIGHT, true);
	_color2.Initialize(SCR_WIDTH, SCR_HEIGHT, true);

	_pingPong[0].Initialize(SCR_WIDTH, SCR_HEIGHT);
	_pingPong[1].Initialize(SCR_WIDTH, SCR_HEIGHT);
	_finalScene.Initialize(SCR_WIDTH, SCR_HEIGHT);
	
	_baselevel.Load("./models/ravine_level_tree.fbx");
	_crystal.Load("./models/crystal.fbx");

	_screenRect._position_attrib_id = _shaderOrtho.GetAttributeLocation("position");
	_screenRect._texture_attrib_id = _shaderOrtho.GetAttributeLocation("textCoord");
	_screenRect.Create();

	_alpha = 0.0f;
	_alpha_dir = 0.0f;
}

void RavineLevel::Update(float elapsed)
{
	if (_alpha_dir == 0)
	{
		_alpha += 2.0f * elapsed;
		if (_alpha >= 1.0f) {
			_alpha_dir = 1;
			_alpha = 1.0f;
		}
	}
	else if (_alpha_dir == 1)
	{
		_alpha -= 2.0f * elapsed;
		if (_alpha <= 0.0f) {
			_alpha_dir = 0;
			_alpha = 0.0f;
		}
	}
}

void RavineLevel::Draw(glm::mat4 viewProjection)
{
	// render
		// ------
	_color1.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 model = glm::mat4(1.0f);		//identity
		glEnable(GL_DEPTH_TEST);

		_modelShader.Use();
		int gWVP = _modelShader.GetUniformLocation("gVP");
		int gWorld = _modelShader.GetUniformLocation("gWorld");
		int alphaID = _modelShader.GetUniformLocation("alpha");

		_modelShader.SetUniformMatrix(gWVP, viewProjection);
		_modelShader.SetUniformMatrix(gWorld, model);
		_modelShader.SetUniformFloat(alphaID, 1.0f);

		int textureID = _modelShader.GetUniformLocation("gSampler");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture

		// render tree and crystal together

		_baselevel.Render();
		_crystal.Render();
	_color1.Unbind();

	// Render crystal
	_color2.Bind();
	/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_modelShader.SetUniformFloat(alphaID, _alpha);
	_crystal.Render();
	*/
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		model = glm::mat4(1.0f);		//identity
		glEnable(GL_DEPTH_TEST);
		_brightFilter.Use();
		gWVP = _brightFilter.GetUniformLocation("gVP");
		gWorld = _brightFilter.GetUniformLocation("gWorld");
		alphaID = _brightFilter.GetUniformLocation("alpha");

		_brightFilter.SetUniformMatrix(gWVP, viewProjection);
		_brightFilter.SetUniformMatrix(gWorld, model);
		_brightFilter.SetUniformFloat(alphaID, _alpha);

		textureID = _brightFilter.GetUniformLocation("gSampler");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture

		// render tree and crystal together

		_baselevel.Render();
		_crystal.Render();
	_color2.Unbind();

#if 1
	//...
	//Do Gausian Blur
	//https://learnopengl.com/Advanced-Lighting/Bloom
	// --------------------------------------------------
	bool horizontal = true, first_iteration = true;
	unsigned int amount = 10;
	_blurShader.Use();

	for (unsigned int i = 0; i < amount; i++)
	{
		_pingPong[horizontal].Bind();

		int horizontalId = _blurShader.GetUniformLocation("horizontal");
		_blurShader.SetUniformInt(horizontalId, horizontal);

		textureID = _blurShader.GetUniformLocation("image");
		glUniform1i(textureID, 0);				// set the textureID to texture unit 0
		glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture

		glBindTexture(GL_TEXTURE_2D, first_iteration ? _color2.GetColorTexture()
			: _pingPong[!horizontal].GetColorTexture());	// bind the render target texture	

		// RenderQuad
		_screenRect.Draw();

		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;

		_pingPong[horizontal].Unbind();
	}

	// final combine
	_bloomFinal.Use();

	int hdr = _bloomFinal.GetUniformLocation("scene");
	glUniform1i(textureID, 0);				// set the textureID to texture unit 0
	glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
	glBindTexture(GL_TEXTURE_2D, _color1.GetColorTexture());

	int bloomTexture = _bloomFinal.GetUniformLocation("bloomBlur");
	glUniform1i(bloomTexture, 1);				// set the textureID to texture unit 0
	glActiveTexture(GL_TEXTURE1);			// activate the texture unit 0 first before binding texture
	glBindTexture(GL_TEXTURE_2D, _pingPong[!horizontal].GetColorTexture());

	_finalScene.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_screenRect.Draw();
	_finalScene.Unbind();

#endif
	//-----------------------------------------------------------------------------
#if 1
		// Restore frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// activate the texture unit 0 first before binding texture
	_shaderOrtho.Use();

	textureID = _shaderOrtho.GetUniformLocation("texture");
	glUniform1i(textureID, 0);				// set the textureID to texture unit 0
	glActiveTexture(GL_TEXTURE0);			// activate the texture unit 0 first before binding texture
	glBindTexture(GL_TEXTURE_2D, _finalScene.GetColorTexture());	// bind the render target texture
	
	_screenRect.Draw();

#endif
}