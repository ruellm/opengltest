#include <stdlib.h>
#include "common.h"
#include "particle_system.h"
#include "Random.h"

/**/
// Shader attribute location
static int POSITION_ATTRIBUTE = 0;
static int COLOR_ATTRIBUTE = 1;
static int SIZE_ATTRIBUTE = 2;

/**/
/**/
ParticleSystem::ParticleSystem()
: _emitter(NULL)
, _flag(0)
, _lifeTime(5.0f)
, _startSpeed(5.0f)
, _startSize(1)
, _maxParticles(100)
, _loop(true)
, _currentTime(0.0f)
, _blendMode (PARTICLE_BLENDMODE_TRANSPARENT)
, _vertices(0)
, _currIdx(0)
, _vertices_size(0)
, _end(false)
{
	_startColor = glm::vec4(1, 1, 1, 1);
}

ParticleSystem::~ParticleSystem()
{
	SAFE_DELETE_ARRAY(_vertices);
}

void ParticleSystem::SetEmitter(ParticleEmitter* emitter)
{
	_emitter = emitter;
}

void ParticleSystem::LoadTexture(const char* szFileName)
{
	_texture.LoadFromAssetDirect( szFileName );
}

void ParticleSystem::Start()
{
	if (_emitter != NULL)
	{
		_emitter->Origin = _position;
		_emitter->speed = _startSpeed;
	}

	for (int i = 0; i < _maxParticles; i++) {
		Particle particle;
		RestartParticle(&particle);
		_particles.push_back(particle);
	}

	const int POSITION_COMPONENT_COUNT = 3;
	const int COLOR_COMPONENT_COUNT = 4;
	const int SIZE_COMPONENT_COUNT = 1;

	const int TOTAL_COMPONENT_COUNT =
		POSITION_COMPONENT_COUNT
		+ COLOR_COMPONENT_COUNT +
		SIZE_COMPONENT_COUNT;

	int currIdx = 0;
	_vertices_size = _particles.size() * TOTAL_COMPONENT_COUNT;
	_vertices = new float[_vertices_size];

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	float vertices[] = {
		// 3D Coordinates	texture coordinates, 
		// in windows, image is upside down, therefore invert texture coordinates
		1.0f,  1.0f, 0.0f,		1.0f, 0.0f,		// top right
		1.0f, -1.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-1.0f,  1.0f, 0.0f,		0.0f, 0.0f,		// top left 
												// second triangle
		1.0f, -1.0f, 0.0f,		1.0f, 1.0f,		// bottom right
		-1.0f, -1.0f, 0.0f,		0.0f, 1.0f,		// bottom left
		-1.0f,  1.0f, 0.0f,		0.0f, 0.0f		// top left
	};

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void ParticleSystem::RestartParticle(Particle* particle)
{
	if (_emitter == NULL) {
		EmitParticlePosition(particle);
	}
	else {
		_emitter->Emit(particle);
	}

	EmitParticle(particle);
}

void ParticleSystem::EmitParticle(Particle* particle)
{
	particle->lifeTime	= RandRange(0, _lifeTime);
	particle->color		= _startColor;
	particle->size		= RandRange(0,_startSize);
	particle->age		= RandRange(0, _lifeTime);
}

void ParticleSystem::EmitParticlePosition(Particle* p)
{
	glm::vec3 unitvec = RandUnitVec();
	p->position = _position;
	p->velocity = unitvec * _startSpeed;
}

bool ParticleSystem::IsOver()
{
    return _end;
}

void ParticleSystem::Update(float elapsed)
{
	_end = false;
	if (!_loop)
	{
		_currentTime += elapsed;
		if (_currentTime >= _duration)
		{
			_end = true;
		}
	}

	_currIdx = 0;

	for (int i = 0; i < _particles.size(); i++) {
		Particle* particle = &_particles[i];

		particle->age += elapsed;
		if (particle->age >= _lifeTime) {
			if (!_end) {
				RestartParticle(particle);
			}
			else {
				_particles.erase(_particles.begin() + i);
			}
		}

		float lifeRatio = (particle->age / particle->lifeTime);
		particle->position += (particle->velocity * elapsed);

		if (_flag & PARTICLE_FLAG_COLOR_OVER_LIFE_TIME)
			particle->color = _colorInterpolator.GetValue(lifeRatio);
		else
			particle->color = _startColor;

		if (_flag & PARTICLE_FLAG_SIZE_OVER_LIFE_TIME) {
			particle->size = _sizeInterpolator.GetValue(lifeRatio);
		}
		else
			particle->size = _startSize;

	}
}

void ParticleSystem::SetAttribute(int position, int color, int size)
{
	POSITION_ATTRIBUTE = position;
	COLOR_ATTRIBUTE = color;
	SIZE_ATTRIBUTE = size;
}

void ParticleSystem::Render(glm::vec3 campos, GLShaderFx* shader)
{
    _texture.Bind();

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	switch (_blendMode) {
	case PARTICLE_BLENDMODE_ONE:
		glBlendFunc(GL_ONE, GL_ONE);	// additive blending
		break;
	case PARTICLE_BLENDMODE_ADDITIVE:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	default:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}
	
	for (int i = 0; i < _particles.size(); i++) {
		Particle* particle = &_particles[i];

		//compute billboard
		glm::vec3 object_look = glm::vec3(0, 0, 1);
		glm::vec3 object_right = glm::vec3(1, 0, 0);

		campos = glm::vec3(campos.x, 0, campos.z);

		glm::vec3 direction = campos - particle->position;
		direction = glm::normalize(direction);

		//get the angle between new look and old look
		float dot = glm::dot(object_look, direction);
		float angle = glm::acos(dot);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 scale = glm::mat4(1.0f);
		scale = glm::scale(scale, glm::vec3(particle->size, particle->size, 1.0f));
		model = glm::translate(model, particle->position);
		
		glm::vec3 newlook = direction;
		glm::vec3 newright = glm::normalize(glm::cross(newlook, glm::vec3(0, 1, 0)));

		model[0][0] = newright.x;
		model[1][0] = newright.y;
		model[2][0] = newright.z;

		model[0][2] = newlook.x;
		model[1][2] = newlook.y;
		model[2][2] = newlook.z;

		int worldID = shader->GetUniformLocation("gWorld");
		shader->SetUniformMatrix(worldID, model * scale);

		int colorID = shader->GetUniformLocation("vColor");
		shader->SetUniformVector4(colorID,particle->color);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE);

}


/**/
/**/
CloudParticle::CloudParticle()
	: ParticleSystem()
{
	_startSize = 100;
	_maxParticles = 100;
	_lifeTime = 2.0f;
	_position = glm::vec3(0, 0, 0);
	_startSpeed = 0;
	_startColor = glm::vec4(1, 1, 1, 1);
}

CloudParticle::~CloudParticle()
{}

void CloudParticle::EmitParticlePosition(Particle* p)
{
	float thickness = 1.0f;
	float X = RandRange(-thickness, thickness);
	float Y = RandRange(-thickness, thickness);
	float Z = RandRange(-thickness, thickness);

	p->position = _position + glm::vec3(X, Y, Z);
	p->velocity = glm::vec3(0, 0, 0) * _startSpeed;
}

/**/
FireParticle::FireParticle()
	: ParticleSystem()
{
	_startSize = 1;
	_maxParticles = 100;
	_lifeTime = 2.0f;
	_position = glm::vec3(0, 0, 0);
	_startColor = glm::vec4(1, 0.8, 0.3, 1);
	_startSpeed = 1.0f;

	 _colorInterpolator.AddValue(0.05, glm::vec4(1.0f, 0.5f, 0.18, 1));
	 _colorInterpolator.AddValue(0.8, glm::vec4(1.0f, 0.0f, 0.0, 1));
	 _colorInterpolator.AddValue(0.9, glm::vec4(0.01f, 0.01f, 0.01, 1));
	 
	_flag |= PARTICLE_FLAG_COLOR_OVER_LIFE_TIME;

	_blendMode = PARTICLE_BLENDMODE_ADDITIVE;
}

void FireParticle::EmitParticlePosition(Particle* p)
{
	float thickness = 2.0f;
	float X = RandRange(-thickness, thickness);
	float Y = RandRange(0, 0);
	float Z = RandRange(-thickness, thickness);

	p->position = _position + glm::vec3(X, Y, Z);
	p->velocity = glm::vec3(0, 0, 0) * _startSpeed;
}

/**/
CircleEffect::CircleEffect()
	: ParticleSystem()
{
	_startSize = 20;
	_maxParticles = 800;
	_lifeTime = 1;
	_position = glm::vec3(0, 0, 0);
	_startColor = glm::vec4(1, 1, 1, 1);
	_startSpeed = 10;

	_sizeInterpolator.AddValue(_lifeTime, 0.0f);
	_sizeInterpolator.AddValue(0.0f, _startSize);
	_flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME;
	
	_blendMode = PARTICLE_BLENDMODE_ADDITIVE;

	_colorInterpolator.AddValue(0.0f, glm::vec4(1, 1, 1, 1));
	_colorInterpolator.AddValue(_lifeTime, glm::vec4(1, 0, 0, 0));
	//_flag |= PARTICLE_FLAG_COLOR_OVER_LIFE_TIME;

	//5 seconds loop animation
	_loop = true;
	_duration = 0.5;
}

void CircleEffect::EmitParticlePosition(Particle* p)
{
	float radius = 5.0f;
	float angle = RandRange(0, 360);
	float angle_rad = glm::radians(angle);

	float X = radius * glm::sin(angle_rad);
	float Y = RandRange(_position.y, _position.y + 1.0f);
	float Z = radius * glm::cos(angle_rad);
	
	p->position = _position + glm::vec3(X, Y, Z);
	p->velocity = glm::vec3(0, 1, 0) * _startSpeed;
}

/**/
HealingEffectStream::HealingEffectStream()
	: ParticleSystem()
	, _angle(0.0f)
{
	_startSize = 20;
	_maxParticles = 500;
	_lifeTime = 1;
	_position = glm::vec3(0, 0, 0);			// center position
	//_startColor = glm::vec4(0, 1, 1, 1);
	_startSpeed = 3;

	_sizeInterpolator.AddValue(_lifeTime, 0.0f);
	_sizeInterpolator.AddValue(0.0f, _startSize);
	_flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME;

	_loop = true;
	_duration = 5;
}

void HealingEffectStream::EmitParticlePosition(Particle* p)
{
	float radius = 1.0f;
	float angle_rad = glm::radians(_angle);

	float X = radius * glm::sin(angle_rad);
	float Y = _position.y;
	float Z = radius * glm::cos(angle_rad);

	p->position = _position + glm::vec3(X, Y, Z);
	p->velocity = glm::vec3(0, 1, 0) * _startSpeed;
}

void HealingEffectStream::Update(float elapsed)
{
	_angle += 4;
	ParticleSystem::Update(elapsed);
}

HealingEffect::HealingEffect() : _angle(0)
{
	position = glm::vec3(0, 0, 0);
}

void HealingEffect::Load()
{
	GLenum error = glGetError();

	stream1._position = position;
	stream2._position = position;

	stream1._startColor = glm::vec4(0, 1, 1, 1);
	stream2._startColor = glm::vec4(0, 1, 1, 1);

	stream1._angle = 0;
	stream2._angle = 180;

	stream1.Start();
	//stream2.Start();

	error = glGetError();

	float size = 1.0f;

	//prepare the ground rune
	float vertices[] = {
		size, 0.0f,  size, 	1.0f, 0.0f,		// top right
		size, 0.0f, -size, 	1.0f, 1.0f,		// bottom right
		-size, 0.0f, size,	0.0f, 0.0f,		// top left 
											// second triangle
		size, 0.0f, -size, 	1.0f, 1.0f,		// bottom right
		-size, 0.0f, -size,	0.0f, 1.0f,		// bottom left
		-size, 0.0f, size,	0.0f, 0.0f		// top left
	};

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	error = glGetError();
	textureStream.LoadFromAssetDirect("./textures/flare/1.png");
	texture.LoadFromAssetDirect("./textures/runes.png");

}

#if 0
void HealingEffect::Update(float elapsed)
{
	stream1.Update(elapsed);
//	stream2.Update(elapsed);

	_angle++;
}

void HealingEffect::Render()
{
	particleShader->Use();

	int textureID = particleShader->GetUniformLocation("gSampler");
	glUniform1i(textureID, 0);				// set the textureID to texture unit 0
	textureStream.Bind();

	stream1.Render();
	//stream2.Render();

	runeShader->Use();
	textureID = runeShader->GetUniformLocation("texture");
	glUniform1i(textureID, 0);				// set the textureID to texture unit 0
	texture.Bind();
	
	int modelID = runeShader->GetUniformLocation("model");
	glm::mat4 model(1.0f);
	model = glm::rotate(model, glm::radians(_angle), glm::vec3(0, 1, 0));
	runeShader->SetUniformMatrix(modelID, model);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
}
#endif

/**/
SparkleEffect::SparkleEffect()
{
	_startSize = 25;
	_maxParticles = 5;
	_lifeTime = 1.0f;
	_position = glm::vec3(0, 0, 0);
	_startSpeed = 0;	
}

void SparkleEffect::EmitParticlePosition(Particle* p)
{
	float thickness = 5.0f;
	float X = RandRange(-thickness, thickness);
	float Y = RandRange(-thickness, thickness);
	float Z = RandRange(-thickness, thickness);

	p->position = _position + glm::vec3(X, Y, Z);
	p->velocity = glm::vec3(0, 0, 0) * _startSpeed;

	_sizeInterpolator.AddValue(0.0f, _startSize);
	_flag |= PARTICLE_FLAG_SIZE_OVER_LIFE_TIME;

	float R = RandRange(0, 1);
	float G = RandRange(0, 1);
	float B = RandRange(0, 1);
	float A = RandRange(0, 1);
	_startColor = glm::vec4(R, G, B, A);

	_colorInterpolator.AddValue(0.0f, glm::vec4(R, G, B, 1));
	_colorInterpolator.AddValue(_lifeTime, glm::vec4(R, G, B, 0));
	_flag |= PARTICLE_FLAG_COLOR_OVER_LIFE_TIME;
}