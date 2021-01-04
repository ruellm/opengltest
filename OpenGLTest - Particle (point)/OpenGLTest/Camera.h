#pragma once

#include "common.h"

typedef enum CAMERA_TYPE
{
	CAMERA_TYPE_LAND,
	CAMERA_TYPE_AIR
}CAMERA_TYPE;

class Camera
{
public:
	Camera(void);
	~Camera(void);

	void Update();

	void LookAt(const glm::vec3& pos, 
		const glm::vec3& up, 
		const glm::vec3& look);		//NOTE: The lookup in camera is NOT normalize vector

	void Walk( float amount, CAMERA_TYPE type = CAMERA_TYPE_AIR );
	void Strafe (float amount, CAMERA_TYPE type = CAMERA_TYPE_AIR);
	void Pitch(float amount);
	void Yaw(float amount);
	void Fly (float amount);

	// just for testing
	void RotatePosition(const glm::vec3& axis, float amount);

	inline glm::mat4 GetViewMatrix() const {
		return m_viewmat;
	}

	inline glm::vec3 GetPosition() const {
		return m_position;
	}

	inline glm::vec3 GetLook() const {
		return  m_look;
	}

	inline glm::vec3 GetRight() const {
		return m_right;
	}

private:
	glm::vec3 m_position;
	glm::vec3 m_up;
	glm::vec3 m_look;
	glm::vec3 m_right;
	glm::mat4 m_viewmat;

	void UpdateViewMtx();

};

