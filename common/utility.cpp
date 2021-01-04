//
// Created by magpayor on 10/17/2017.
//
#include <algorithm>    // std::max
#include "utility.h"

void CreateRectangle(int size, 
	int position_attrib_id,
	int texture_attrib_id,
	GLuint* VBO
#ifndef OPENGLES2
	, GLuint* VAO
#endif
	)
{
    float vertices[] = {
		size, 0.0f,  size, 	1.0f, 0.0f,		// top right
		size, 0.0f, -size, 	1.0f, 1.0f,		// bottom right
		-size, 0.0f, size,	0.0f, 0.0f,		// top left
											// second triangle
		size, 0.0f, -size, 	1.0f, 1.0f,		// bottom right
		-size, 0.0f, -size,	0.0f, 1.0f,		// bottom left
		-size, 0.0f, size,	0.0f, 0.0f		// top left
	};
	
#ifndef OPENGLES2
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);
#endif

	glGenBuffers(1, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(position_attrib_id, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(position_attrib_id);

	glVertexAttribPointer(texture_attrib_id, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(texture_attrib_id);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifndef OPENGES2
	glBindVertexArray(0);
#endif

}

/*--------------------------------------------------------------*/
// Watch Gamedev.net line geometry testing for explanation
// We must decide to put this somehwere
// this is using SLAB testing
// ZBU_TODO: migrate/transfer to common Math handler class

bool RaySlabIntersect(float slabmin, float slabmax, float raystart, float rayend, float& tbenter, float& tbexit)
{
	float raydir = rayend - raystart;

	// ray parallel to the slab
	if (fabs(raydir) < 1.0E-9f)
	{
		// ray parallel to the slab, but ray not inside the slab planes
		if(raystart < slabmin || raystart > slabmax)
		{
			return false;
		}
			// ray parallel to the slab, but ray inside the slab planes
		else
		{
			return true;
		}
	}

	// slab's enter and exit parameters
	float tsenter = (slabmin - raystart) / raydir;
	float tsexit = (slabmax - raystart) / raydir;

	// order the enter / exit values.
	if(tsenter > tsexit)
	{
		//swapf(tsenter, tsexit);
		float temp = tsenter;
		tsenter = tsexit;
		tsexit = temp;
	}

	// make sure the slab interval and the current box intersection interval overlap
	if (tbenter > tsexit || tsenter > tbexit)
	{
		// nope. Ray missed the box.
		return false;
	}
		// yep, the slab and current intersection interval overlap
	else
	{
		// update the intersection interval
		tbenter = std::max(tbenter, tsenter);
		tbexit = std::min(tbexit, tsexit);
		return true;
	}
}/*--------------------------------------------------------------*/

glm::vec3 GetCenter(glm::vec3 min, glm::vec3 max)
{
	float cx = min.x + ((max.x-min.x) / 2.0f);
	float cy = min.y + ((max.y-min.y) / 2.0f);
	float cz = min.z + ((max.z-min.z) / 2.0f);
	return glm::vec3(cx, cy, cz);
}
