//
// Created by magpayor on 10/17/2017.
//

#ifndef RAMO_UTILITY_H
#define RAMO_UTILITY_H
#include "common.h"

void CreateRectangle(int size,
	int position_attrib_id,
	int texture_attrib_id,
	GLuint* VBO
#ifndef OPENGLES2
	, GLuint* VAO
#endif
);

bool RaySlabIntersect(float slabmin, float slabmax, float raystart, float rayend, float& tbenter, float& tbexit);
glm::vec3 GetCenter(glm::vec3 min, glm::vec3 max);

#endif //RAMO_UTILITY_H
