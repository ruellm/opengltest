#pragma once
#include <stdlib.h>

class RandomValue
{
public:
	RandomValue() : _min(0), _max(0) {}
	RandomValue(float min, float max) 
		: _min(min), _max(max) { }

	~RandomValue() {}

	float GetRandomValue() {
		return _min + (rand() / (RAND_MAX / (_max - _min)));
	}

	inline void Set(float min, float max) {
		_min = min;
		_max = max;
	}

	float _min;
	float _max;
};