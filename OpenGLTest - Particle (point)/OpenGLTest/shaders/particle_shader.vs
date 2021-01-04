layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec4 vColor;

uniform mat4 gVP;
uniform mat4 gWorld;

out vec4 Color;	

void main()                                                                   
{                                                                     
	 gl_Position =   gVP *  gWorld * vec4(vPosition, 1.0);
	 gl_PointSize = 50.0;
	 Color = vColor;
}
//---

