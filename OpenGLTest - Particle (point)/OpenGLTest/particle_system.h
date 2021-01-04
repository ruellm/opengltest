/**
* Common particle system definition
* No repeat (no continous emmitter)
* Date Created: November 6, 2017
* Author: ruellm@yahoo.com
*/

#pragma once

#include "common.h"
#include "Particle.h"
#include "recycling_array.h"

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	virtual void Update(float elapsed);
	virtual void Generate(int count);
	void Render();
	void Cleanup();
	
	// particle system attributes
	glm::vec3	_position;				// position of emitter
	float		_gravity;
	int			_max_count;				// maximum count for emmitters, total count non emmitters

	float		_max_life_time;			// maximimum life time of each particle
	bool		_life_flag;				// does the particle has different life time (randomize)?
	
	float		_max_size;				// size of particle
	float		_min_size;				// use for randomization
	bool		_size_flag;				// has size difference (random)?

	glm::vec3	_min_direction;			// minimum direction vector used for randomization
	glm::vec3   _max_direction;			// maximum direction vector for randomization

	glm::vec3	_emit_radius;			// location clip

	glm::vec4	_color;					
		
	//randmomize parameter
	//use for constant moving particle
	// because of X randmize direction, we might not need direction?
	//TODO: clean this up later
	bool _direction_ramdomized[3];
	RandomValue _direction_x_random;
	RandomValue _direction_y_random;
	RandomValue _direction_z_random;

protected:
	void CreateParticle(int count);

	// this is the master list, sub class can allocate the 
	// particles and implement New and Delete interfaces
	std::vector<Particle*> _particles;				
	virtual void DeleteParticle(Particle* p);
	virtual Particle* GetNewParticle();
};

/**/
/**/
class ParticleEmmitter : public ParticleSystem
{
public:
	ParticleEmmitter();
	~ParticleEmmitter();

	virtual void Update(float elapsed);
	virtual void Generate(int count);
	virtual Particle* GetNewParticle();
	virtual void DeleteParticle(Particle* p);

private:
	RecyclingArray<Particle>	_reserved;
};

