
#include "scene_object.h"
//#include "utility.h"

SceneObject::SceneObject() 
	: m_angleX(0.0f),
	m_angleY(0.0f), m_angleZ(0.0f),
	m_bIsVisible(true)
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

