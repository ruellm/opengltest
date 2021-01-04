#include "trail_particle.h"

static int current_node = 0;
static int current_max = 0;

static float DELAY_FACTOR = 0.25;

TrailParticle::TrailParticle() 
	: _trail_node(NULL), _tail_level(1), _tail_degrade(2)
{
}


TrailParticle::~TrailParticle()
{
	SAFE_DELETE_ARRAY(_trail_node);
}


void TrailParticle::Update(float elapsed)
{
	float current_delay = 1.0f;
	float SPEED = 0.20f;
	elapsed = 0.1f;

	glm::vec3 direction = _target - _position;
	direction = glm::normalize(direction);
	_position += (direction* SPEED * elapsed);

#if 1
	for (int t = 0; t < _tail_level; t++) {

		for (int c = 0; c < _trail_node[t].size(); c++) {
			Particle* pa = _trail_node[t].at(c);

			glm::vec3 dir = glm::normalize(_position - pa->_position);

		//	pa->_age += elapsed;
		///	if (pa->_age >= pa->_life_time) {
		//		pa->_alive = false;
		//		continue;
		//	}

			dir *= current_delay;
			pa->_position.x += (dir.x * SPEED * elapsed);
			
			if (t == 0)
				pa->_color = glm::vec4(1, 0, 0, 1);
			else if(t == 1)
				pa->_color = glm::vec4(1, 1, 0, 1);
			else if (t == 2)
				pa->_color = glm::vec4(1, 1, 1, 1);
			else if (t == 3)
				pa->_color = glm::vec4(0, 1, 0, 1);
			else if (t == 4)
				pa->_color = glm::vec4(0, 0, 1, 1);
		}

		current_delay -= DELAY_FACTOR;
	}
#endif
}

void TrailParticle::Generate(int count)
{
	_head_particle_count = count;

	_trail_node = new ParticleList[_tail_level];
	current_max = count;

	_min_direction = glm::vec3(0, 0, 0);
	_max_direction = glm::normalize(_goto_direction);

	DELAY_FACTOR = 1.0f / _tail_level;
	
	for (int t = 0; t < _tail_level; t++) {
		CreateParticle(current_max);
		current_max = current_max / _tail_degrade;
		current_node++;
	}
}

Particle* TrailParticle::GetNewParticle()
{
	Particle* p = new Particle();

	_trail_node[current_node].push_back(p);

	return p;
}

void TrailParticle::DeleteParticle(Particle* p)
{
	// search this particle
	for (int t = 0; t < _tail_level; t++) {
		for (int c = 0; c < _trail_node[t].size(); c++)
		{
			Particle* pa = _trail_node[t].at(c);
			if (pa == p)
				SAFE_DELETE(p);
			_trail_node[t].erase(_trail_node[t].begin() + c);
		}
	}
}
