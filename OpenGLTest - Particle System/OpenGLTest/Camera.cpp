#include "Camera.h"


Camera::Camera(void)
{
	//imagime your right hand, thumb pointing to the right, index up, the rest towards you
	m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	m_look = glm::vec3(0.0f, 0.0f, 1.0f);
	m_right = glm::vec3(1.0f, 0.0f, 0.0f);

	m_viewmat = glm::mat4(1.0f);
}


Camera::~Camera(void)
{
}

void Camera::UpdateViewMtx()
{
#if 1
	// align axises
	m_look = glm::normalize(m_look);
	m_up = glm::vec3(0,1,0);	// For FPS camera
	//m_up = glm::normalize(m_up);
	m_right = glm::normalize(glm::cross(m_look, m_up));
	m_up = glm::cross(m_right, m_look);

	m_viewmat[0][0] = m_right.x;
	m_viewmat[1][0] = m_right.y;
	m_viewmat[2][0] = m_right.z;
	m_viewmat[0][1] = m_up.x;
	m_viewmat[1][1] = m_up.y;
	m_viewmat[2][1] = m_up.z;
	m_viewmat[0][2] =-m_look.x;
	m_viewmat[1][2] =-m_look.y;
	m_viewmat[2][2] =-m_look.z;
	m_viewmat[3][0] =-glm::dot(m_right, m_position);
	m_viewmat[3][1] =-glm::dot(m_up, m_position);
	m_viewmat[3][2] = glm::dot(m_look, m_position);
#else
	m_viewmat = glm::lookAt(m_position, m_position+m_look, glm::vec3(0,1,0));
#endif
	
}

void Camera::Update()
{
	UpdateViewMtx();
}

void Camera::LookAt(const glm::vec3& pos, 
		const glm::vec3& up, const glm::vec3& look)	   
		//NOTE: The lookup in camera is NOT normalize vector
{
	m_look = glm::normalize(look - pos);
	m_up = glm::normalize(up);
	m_position = pos;

	Update();
}

void Camera::Walk( float amount, CAMERA_TYPE type )
{
	if( type == CAMERA_TYPE_LAND ) {
		m_position += (glm::vec3(m_look.x, 0, m_look.z) * amount);
	} else {
		m_position += (m_look * amount);
	}	
}

void Camera::Strafe (float amount, CAMERA_TYPE type)
{
	if( type == CAMERA_TYPE_LAND ) {
		m_position += (glm::vec3(m_right.x, 0, m_right.z) * amount);
	} else {
		m_position += (m_right * amount);
	}
}

void Camera::Fly (float amount)
{
	m_position += (m_up * amount);
}

void Camera::Pitch(float amount)
{
#if 0	
	//--------------------------------------
	// ROtation using Vector axis
	//--------------------------------------
	m_look = glm::rotateX(m_look, amount);
	m_right = glm::rotateX(m_up, amount);

	m_look = glm::normalize(m_look);
	m_up = glm::normalize(m_up);

#else
	//--------------------------------------
	// ROtation using matrix
	//--------------------------------------
	glm::mat4 rotmatx = glm::mat4(1.0f);
	rotmatx = glm::rotate(rotmatx, amount, m_right);

	glm::vec4 transformdLook = glm::normalize( rotmatx * glm::vec4(m_look,1.0f) );
	glm::vec4 transformdUp = glm::normalize( rotmatx * glm::vec4(m_up,1.0f) );

	m_look = glm::vec3(transformdLook.x, transformdLook.y, transformdLook.z);
	m_up =  glm::vec3(transformdUp.x, m_up.y, transformdUp.z); // pitch should not rotate the Y coord
#endif

}

void Camera::Yaw(float amount)
{
#if 0	
	///m_look = glm::rotate(m_look, amount, m_up);
	//m_right = glm::rotate(m_right, amount, m_up);

	m_look = glm::rotateY(m_look, amount);
	m_right = glm::rotateY(m_right, amount);

	m_look = glm::normalize(m_look);
	m_right = glm::normalize(m_right);
#else

	glm::mat4 rotmatx = glm::mat4(1.0f);
	rotmatx = glm::rotate(rotmatx, amount, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec4 transformdLook = glm::normalize( rotmatx * glm::vec4(m_look,1.0f) );
	glm::vec4 transformdRight = glm::normalize( rotmatx * glm::vec4(m_right,1.0f) );

	m_look = glm::vec3(transformdLook.x, transformdLook.y, transformdLook.z);
	m_right =  glm::vec3(transformdRight.x, transformdRight.z, transformdRight.z);

#endif
}

void Camera::RotatePosition(const glm::vec3& axis, float amount)
{
	glm::mat4 rotmatx = glm::mat4(1.0f);
	rotmatx = glm::rotate(rotmatx, amount, axis);

	glm::vec4 newpos = rotmatx * glm::vec4(m_position,1.0f);
    glm::vec3 look = axis;
	m_position = glm::vec3(newpos);
	m_look = glm::normalize(look - m_position);
}

