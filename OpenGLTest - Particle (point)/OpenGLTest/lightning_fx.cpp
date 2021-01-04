#include "lightning_fx.h"

static int state = 0; // 0: none, 1: showing in, 2: showing out, 3 : done, 4: stay 
static float last_time = 0;

LightningBolt::LightningBolt() : AnimatedSprite(),
	_alphaSpeedIn (10.0f), _alpha(0.0f),_alphaSpeedOut (5.0f),
	_staySeconds(1.0f)
{
	//...
}

LightningBolt::~LightningBolt()
{
	///...
}

float LightningBolt::GetAlpha()
{ 
	return _alpha;
}

void LightningBolt::SetCameraPos(glm::vec3 campos)
{
	_campos = campos;
}

void LightningBolt::SetAlphaSpeed(float in, float out)
{
	_alphaSpeedIn = in;
	_alphaSpeedOut = out;
}

void LightningBolt::SetStaySeconds(float sec)
{
	_staySeconds = sec;
}

void LightningBolt::Update(float elapsed)
{
	if (state == 0) return;

	AnimatedSprite::Update(elapsed);
	SceneObject::Update(elapsed);

	glm::vec3 campos = glm::vec3(_campos.x, 0, _campos.z);

	glm::vec3 direction = campos - GetPosition();
	direction = glm::normalize(direction);

	glm::vec3 object_look = glm::vec3(m_worldMat[0][2], m_worldMat[1][2], m_worldMat[2][2]);

	//get the angle between new look and old look
	float dot = glm::dot(object_look, direction);
	float angle = glm::acos(dot);

	glm::mat4 model = glm::mat4(1.0f);

	glm::vec3 newlook = direction;
	glm::vec3 newright = glm::normalize(glm::cross(newlook, glm::vec3(0, 1, 0)));

	m_worldMat[0][0] = newright.x;
	m_worldMat[1][0] = newright.y;
	m_worldMat[2][0] = newright.z;

	m_worldMat[0][2] = newlook.x;
	m_worldMat[1][2] = newlook.y;
	m_worldMat[2][2] = newlook.z;

	if (state == 1) {
		_alpha += (_alphaSpeedIn * elapsed);
		if (_alpha >= 1.0f) {
			_alpha = 1.0f;
			state = 4;

			//TODO: need to change this
			last_time = glfwGetTime();
		}
	}
	else if(state == 2) {
		_alpha -= (_alphaSpeedOut * elapsed);
		if (_alpha <= 0.0f)
		{
			_alpha = 0.0f;
			state = 3;
		}
	}
	else if (state == 4) {
		float current_time = glfwGetTime();
		float diff = current_time - last_time;

		if (diff > _staySeconds) {
			state = 2;
		}
	}
}

void LightningBolt::Draw()
{
	AnimatedSprite::Draw();
}

void LightningBolt::Show()
{
	state = 1;
}

int LightningBolt::GetStatus()
{
	return state;
}