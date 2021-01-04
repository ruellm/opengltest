#pragma once
#include "common.h"

//
// Base Class for objects in the scene
// ex: model3d, lights, reference points
//
class SceneObject
{
public:
	SceneObject();
	virtual ~SceneObject();

	virtual void Update(float elapsed);
	virtual void Draw();

	void Move( glm::vec3& dir, float value );
	void RotateX( float value );
	void RotateY( float value );
	void RotateZ( float value );

	void ScaleX( float value );
	void ScaleY( float value );
	void ScaleZ( float value );

	inline void SetPosition( glm::vec3 position ) {
		m_position = position; 	
	}
	
	inline glm::mat4& GetWorldMatrix( ) {
		return m_worldMat;
	}

	inline glm::vec3 GetPosition( ) {
		return 	m_position;
	}

	inline glm::vec3& GetScale( ) {
		return m_scale;
	}

	inline void GetAngles(float* x, float* y, float* z) {
		*x = m_angleX;
		*y = m_angleY;
		*z = m_angleZ;
	}

	inline void SetAngles(float x, float y, float z) {
		m_angleX = x;
		m_angleY = y;
		m_angleZ = z;
	}

	inline void SetScale(float x, float y, float z)	 {
		m_scale.x = x;
		m_scale.y = y;
		m_scale.z = z;
	}

	inline void ResetScale( ) {
		m_scale = glm::vec3( 1.0f, 1.0f, 1.0f );
	}

	inline void SetVisible( bool value ) {
		m_bIsVisible = value;
	}

	inline bool IsVisible ( ) const {
		return m_bIsVisible;
	}

    inline void SetID( int id ) {
        _id = id;
    }
    inline int GetID( ) const{
        return _id;
    }

protected:
	glm::vec3 m_position;
	float m_angleX;
	float m_angleY;
	float m_angleZ;

	glm::mat4 m_worldMat;

	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_look;

	glm::vec3 m_scale;

	bool m_bIsVisible;
    int _id;
};

/**/
/**/
class PickableObject : public SceneObject
{
public:
    PickableObject();

	void GetBoundingBox(glm::vec3* min, glm::vec3* max);

    virtual bool MousePick( const glm::vec3& rayPos,
                            const glm::vec3& rayDir, glm::vec3* hitPos );
protected:
	virtual void GenerateBoundingBox();

    // bounding box definitions
    glm::vec3 m_min;
    glm::vec3 m_max;
};
