#pragma once
#include "particle_system.h"

typedef std::vector<Particle*> ParticleList;

class TrailParticle : public ParticleSystem
{
public:
	TrailParticle();
	~TrailParticle();

	virtual void Update(float elapsed);
	virtual void Generate(int count);
	virtual Particle* GetNewParticle();
	virtual void DeleteParticle(Particle* p);

	int				_tail_level;
	glm::vec3		_goto_direction;			//normalized direction vector
	glm::vec3		_target;
	int				_tail_degrade;
private:
	int				_head_particle_count;
	ParticleList*	_trail_node;

};

