
#include "scene_object.h"
#include "utility.h"

SceneObject::SceneObject() 
	: m_angleX(0.0f),
	m_angleY(0.0f), m_angleZ(0.0f),
	m_bIsVisible(true), _id(-1)
{
	m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_look = glm::vec3(0.0f, 0.0f, 1.0f);
	m_right = glm::vec3(1.0f, 0.0f, 0.0f);

	m_worldMat = glm::mat4(1.0f);

	m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

SceneObject::~SceneObject() 
{

}
	 
void SceneObject::Update(float elapsed)
{
	glm::mat4 matScale = glm::mat4(1.0f);
	matScale = glm::scale( matScale, m_scale );

	glm::mat4 matPos = glm::mat4(1.0f);
    matPos =  glm::translate(matPos, m_position );
	
	glm::mat4 matRotX = glm::mat4(1.0f);
	matRotX = glm::rotate(matRotX, m_angleX, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 matRotY = glm::mat4(1.0f);
	matRotY = glm::rotate(matRotY, m_angleY, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 matRotZ = glm::mat4(1.0f);
	matRotZ = glm::rotate(matRotZ, m_angleZ, glm::vec3(0.0f, 0.0f, -1.0f));

	m_worldMat = glm::mat4(1.0f);
	m_worldMat= matPos * (matRotX * matRotY * matRotZ) * matScale;
}

void SceneObject::Draw()
{
	//...
}

void SceneObject::Move( glm::vec3& dir, float value )
{
	dir = glm::normalize( dir );
	m_position += (dir * value);
}

void SceneObject::RotateX( float value )
{
	value = glm::radians( value );
	m_angleX += value;

}

void SceneObject::RotateY( float value )
{
	value = glm::radians( value );
	m_angleY += value;
}

void SceneObject::RotateZ( float value )
{
	value = glm::radians( value );
	m_angleZ += value;
}

void SceneObject::ScaleX( float value )
{
	m_scale.x += value;
}

void SceneObject::ScaleY( float value )
{
	m_scale.y += value;

}

void SceneObject::ScaleZ( float value )
{
	m_scale.z += value;
}

/**/
/**/
/**/
PickableObject::PickableObject()
        : SceneObject()
{
    //...
}

void PickableObject::GenerateBoundingBox()
{
	//...
}

bool PickableObject::MousePick( const glm::vec3& rayPos,
                        const glm::vec3& rayDir, glm::vec3* hitPos )

{
    glm::vec4 min = m_worldMat * glm::vec4(m_min, 1 );
    glm::vec4 max = m_worldMat * glm::vec4(m_max, 1 );
    glm::vec3 rayEnd = 	rayPos + (rayDir * 1000.0f);		   // Multiplied to Far clip plane

    // initialise to the segment's boundaries.
    float tenter = 0.0f, texit = 1.0f;

    // test X slab
    if (!RaySlabIntersect(min.x, max.x, rayPos.x, rayEnd.x, tenter, texit))
    {
        return false;
    }

    // test Y slab
    if (!RaySlabIntersect(min.y, max.y, rayPos.y, rayEnd.y, tenter, texit))
    {
        return false;
    }

    // test Z slab
    if (!RaySlabIntersect(min.z, max.z, rayPos.z, rayEnd.z, tenter, texit))
    {
        return false;
    }

    // all intersections in the green. Return the first time of intersection, tenter.
    //tinter = tenter;
    return  true;
}

void PickableObject::GetBoundingBox(glm::vec3* min, glm::vec3* max)
{
	*min = m_min;
	*max = m_max;
}