#pragma once


// GLFW and GLAD
#include <glad/glad.h>      // glad is a loader library needed for opengl to work, alternative GLEW
// include glad.c in the source, file is generated at http://glad.dav1d.de/
#include <GLFW\glfw3.h>

// standard STL
#include <vector>
#include <string>
#include <map>

// GLM!!
// vec3, vec4, ivec4, mat4 
#include <glm/glm.hpp> 
// value_ptr
#include <glm/gtc/type_ptr.hpp>

// translate, rotate, scale, perspective 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>

#if 0
// Matrix elements access
#include <glm/gtc/matrix_access.hpp>
// rotate vector
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>


#include <glm/gtc/quaternion.hpp> 
#include <glm\gtx\intersect.hpp>
#endif

#define SAFE_DELETE( obj ) { if( obj ) { delete obj; obj = NULL; } }
#define SAFE_DELETE_ARRAY( obj ) { if( obj ) { delete[] obj; obj = NULL; } }