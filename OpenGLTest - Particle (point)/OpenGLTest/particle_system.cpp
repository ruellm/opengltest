#include <stdlib.h>
#include "particle_system.h"

double randMToN(double M, double N)
{
	return M + (rand() / (RAND_MAX / (N - M)));
}

ParticleSystem::ParticleSystem() 
	: _gravity (0.0f),		 _max_count(0), 
	_max_life_time(1.0f),	 _life_flag(false),
	_max_size(1.0f),		_min_size(0.0f), 
	_size_flag(false)
{
	_position		= glm::vec3(0, 0, 0);
	_min_direction	= glm::vec3(-1.0f, -1.0f, -1.0f);
	_max_direction	= glm::vec3(1.0f, 1.0f, 1.0f);
	_emit_radius	= glm::vec3(1.0f, 1.0f, 1.0f);
	_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}


ParticleSystem::~ParticleSystem()
{
	Cleanup();
}

void ParticleSystem::Cleanup()
{
	for (int i = 0; i < _particles.size(); i++) {
		DeleteParticle(_particles.at(i));
	}

	_particles.empty();
}

void ParticleSystem::Update(float elapsed)
{
	std::vector<Particle*>::iterator it = _particles.begin();

	while (it != _particles.end()) {
		Particle* part = *it;
		part->Update(elapsed);

		if (!part->IsAlive()) {
			DeleteParticle(part);
		}
		else {
			// add gravity
			part->_direction += glm::vec3(0, _gravity * elapsed, 0);
		}

		it++; 
	}

	it = _particles.begin();
	while (it != _particles.end()) {
		Particle* part = *it;
	
		if (!part->IsAlive()) {
			_particles.erase(it);
			it = _particles.begin();
		}
		else
		{
			it++;
		}
	}
}

void ParticleSystem::Render()
{
	const int POSITION_COMPONENT_COUNT = 3;
	const int COLOR_COMPONENT_COUNT = 4;
	const int TOTAL_COMPONENT_COUNT = 
		POSITION_COMPONENT_COUNT
		+ COLOR_COMPONENT_COUNT;

	int currIdx = 0;
	int vertices_size = _particles.size() * TOTAL_COMPONENT_COUNT;
	float* vertices = new float[vertices_size];

	for (int i = 0; i < _particles.size(); i++) {
		Particle* part = _particles.at(i);
		vertices[currIdx++] = part->_position.x;
		vertices[currIdx++] = part->_position.y;
		vertices[currIdx++] = part->_position.z;

		vertices[currIdx++] = part->_color.r;
		vertices[currIdx++] = part->_color.g;
		vertices[currIdx++] = part->_color.b;
		vertices[currIdx++] = part->_color.a;
	}

	// ATTENTION! this approach is soooooo fucking slow
	// we should not create vertex buffers every frame
	GLuint VBO, VAO;
	GLenum error;

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_size * sizeof(float), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_POINTS, 0, _particles.size());

	SAFE_DELETE_ARRAY(vertices);
}

void ParticleSystem::Generate(int count)
{
	_max_count = count;
	CreateParticle(_max_count);
}

void  ParticleSystem::CreateParticle(int count)
{
	for (int i = 0; i < count; i++) {
		Particle* part = GetNewParticle();

		if (part == NULL) continue;

		// does our system allows different lifetime particle?
		if (_life_flag)
		{
			part->_life_time = randMToN(0, _max_life_time);
		}
		else {
			part->_life_time = _max_life_time;
		}

		// does our system allows different different size particle?
		if (_size_flag)
		{
			part->_size = randMToN(_min_size, _max_size);
		}
		else {
			part->_size = _max_size;
		}

		//determine a random victor between max and min direction
		float fRandX = randMToN(_min_direction.x, _max_direction.x);
		float fRandY = randMToN(_min_direction.y, _max_direction.y);
		float fRandZ = randMToN(_min_direction.z, _max_direction.z);

		part->_direction_ramdomized[0] = _direction_ramdomized[0];
		part->_direction_ramdomized[1] = _direction_ramdomized[1];
		part->_direction_ramdomized[2] = _direction_ramdomized[2];

		part->_direction_x_random.Set(_direction_x_random._min, _direction_x_random._max);
		part->_direction_y_random.Set(_direction_y_random._min, _direction_y_random._max);
		part->_direction_z_random.Set(_direction_z_random._min, _direction_z_random._max);

		part->_direction = glm::vec3(fRandX, fRandY, fRandZ);

		//part->_direction = glm::normalize(part->_direction);
		part->_position = _position;

		// pick a random vector between +/- emitradius
		fRandX = randMToN(-_emit_radius.x, _emit_radius.x);
		fRandY = randMToN(-_emit_radius.y, _emit_radius.y);
		fRandZ = randMToN(-_emit_radius.z, _emit_radius.z);
		part->_position += glm::vec3(fRandX, fRandY, fRandZ);

		//TODO: extend this later to support multiple color and color step
		part->_color = _color;
		part->_alive = true;
		part->_age = 0;

		// add particle to our list
		_particles.push_back(part);
	}
}

Particle* ParticleSystem::GetNewParticle()
{
	if (_particles.size() + 1 > _max_count)
		return NULL;

	return  new Particle();;
}

void ParticleSystem::DeleteParticle(Particle* p)
{
	SAFE_DELETE(p);
}

/**/
/**/
ParticleEmmitter::ParticleEmmitter()
{
	//...
}

ParticleEmmitter::~ParticleEmmitter()
{
	//...
}

Particle* ParticleEmmitter::GetNewParticle()
{
	return _reserved.GetNewElement();
}

void ParticleEmmitter::DeleteParticle(Particle* p)
{
	_reserved.Delete(p);

}

void ParticleEmmitter::Generate(int count)
{
	_max_count = count;
	_reserved.Generate(count);
}

void ParticleEmmitter::Update(float elapsed)
{
	ParticleSystem::Update(elapsed);
	int free_cnt = _max_count - _particles.size();

	CreateParticle(free_cnt);
}