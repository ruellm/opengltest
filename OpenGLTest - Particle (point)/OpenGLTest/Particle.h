/**
 * Single Particle definition
 * Date Created: November 6, 2017
 * Author: ruellm@yahoo.com
*/
#pragma once
#include "common.h"
#include "RandomValue.h"

class Particle
{
public:
	Particle() : _life_time(1.0f), _age(0.0f), _size(1.0f), _alive(true) { }
	~Particle() {}

	// this function is inline not because it's small, but because it's only called
	// in one place (CParticleEmitter.Update()).  This way we can also dodge the
	// performance hit associated with calling a function many times (remember, we're
	// in a for loop in CParticleEmitter.Update()).

	// your needs may vary.  For example, if you can afford the speed hit, i
	inline void Update(float elapsed) {
		_age += elapsed;

		if (_age >= _life_time) {
			_alive = false;
			return;
		}

		glm::vec3 finaldirection = _direction;
		if (_direction_ramdomized[0])
			finaldirection.x = _direction_x_random.GetRandomValue();
		if (_direction_ramdomized[1])
			finaldirection.y = _direction_y_random.GetRandomValue();
		if (_direction_ramdomized[2])
			finaldirection.z = _direction_z_random.GetRandomValue();

		_position	+= (finaldirection * elapsed );
	}

	inline bool IsAlive() const {
		return _alive;
	}

	glm::vec3	_position;
	glm::vec4	_color;

	glm::vec3	_direction;

	float		_life_time;
	float		_age;
	float		_size;
	bool		_alive;

	//use for constant moving particle
	bool _direction_ramdomized[3];
	RandomValue _direction_x_random;
	RandomValue _direction_y_random;
	RandomValue _direction_z_random;

	//
	//TODO: we might need acceleration speed for each particle!!
	//
};