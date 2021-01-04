layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec4 vColor;
layout (location = 2) in float fSize;

uniform mat4 gVP;
uniform mat4 gWorld;
uniform vec3 u_cameraPos;

out vec4 Color;	

const float minPointScale = 0.1;
const float maxPointScale = 0.7;
const float maxDistance   = 100.0;

void main()                                                                   
{                                                                     
	vec4 positionWorld = gWorld * vec4(vPosition, 1.0);
	gl_Position =   gVP *  positionWorld;
	float cameraDist = distance(positionWorld.xyz, u_cameraPos);
	float pointScale = 1.0 - (cameraDist / maxDistance);
    
	pointScale = max(pointScale, minPointScale);
    pointScale = min(pointScale, maxPointScale);
		 
	gl_PointSize = fSize * pointScale;
	
	Color = vColor;
}
//---

