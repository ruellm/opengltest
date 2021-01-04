/**
* Particle System
* Design influence: emulate Unity's particle engine features
* this is also a port from https://www.3dgep.com/simulating-particle-effects-using-opengl/
* Created: December 31, 2017
* Author: ruellm@yahoo.com
*/

#pragma once
#include "common.h"
#include "Interpolator.h"
#include "texture2D.h"

#define	PARTICLE_FLAG_COLOR_OVER_LIFE_TIME	2
#define PARTICLE_FLAG_SIZE_OVER_LIFE_TIME	4

enum ParticleBlendMode
{
	PARTICLE_BLENDMODE_TRANSPARENT	= 0,
	PARTICLE_BLENDMODE_ADDITIVE		= 1,
	PARTICLE_BLENDMODE_ONE			= 2
};

typedef Interpolator<glm::vec4> ColorInterpolator;
typedef Interpolator<float>		SizeInterpolator;

/**/
struct Particle
{
	glm::vec3	position;
	glm::vec3	velocity;
	glm::vec4	color;
	float		age;
	float		size;
	float		lifeTime;

	Particle() 
		: position(0)
		, velocity(0)
		, color(0)
		, age(0)
		, size(0)
		, lifeTime(0)
	{}
};

/**/
class ParticleEmitter
{
public:
	glm::vec3	Origin;
	float		speed;
	virtual void Emit(Particle* particle) = 0;
};


/**/
class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	void SetAttribute(int position, int color, int size);
	bool IsOver();

	void LoadTexture(const char* szFileName);
	void SetEmitter(ParticleEmitter* emitter);
	virtual void Update(float elapsed);

	void Start();
	void Render();

	float					_lifeTime;
	float					_startSpeed;
	float					_startSize;
	glm::vec4				_startColor;
	int						_maxParticles;
	int						_flag;
	ParticleEmitter*		_emitter;
	glm::vec3				_position;
	
	float					_endSize;
	bool					_loop;
	float					_duration;
	float					_currentTime;
	enum ParticleBlendMode	_blendMode;

protected:
	ColorInterpolator		_colorInterpolator;
	SizeInterpolator		_sizeInterpolator;
	Texture2D				_texture;
	std::vector<Particle>	_particles;
	bool 					_end;

	virtual void EmitParticle(Particle* particle);
	virtual void EmitParticlePosition(Particle* p);

private:
	void RestartParticle(Particle* p);

	float*					_vertices;
	int						_currIdx;
	int						_vertices_size;
	GLuint					VBO, VAO;
};

/**/
class CloudParticle : public ParticleSystem
{
public:
	CloudParticle();
	~CloudParticle();

	virtual void EmitParticlePosition(Particle* p);
};

/**/
class FireParticle : public ParticleSystem
{
public:
	FireParticle();
	virtual void EmitParticlePosition(Particle* p);

};

/**/
class CircleEffect : public ParticleSystem
{
public:
	CircleEffect();
	virtual void EmitParticlePosition(Particle* p);

};

/**/
class HealingEffectStream : public ParticleSystem
{
public:
	HealingEffectStream();
	virtual void EmitParticlePosition(Particle* p);
	virtual void Update(float elapsed);

	float	_angle;
};

#include "GLShaderFx.h"
class HealingEffect
{
public:
	HealingEffect();
	void Load();
	void Update(float elapsed);
	void Render();

	//temporary
	GLShaderFx* particleShader;
	GLShaderFx* runeShader;

private:
	HealingEffectStream stream1;
	HealingEffectStream stream2;

	glm::vec3 position;
	GLuint VBO;
	GLuint VAO;

	Texture2D texture;
	Texture2D textureStream;
	float _angle;
};

/**/
class SparkleEffect : public ParticleSystem
{
public:
	SparkleEffect();
	virtual void EmitParticlePosition(Particle* p);
};