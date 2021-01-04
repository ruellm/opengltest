layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTexCoord;

uniform mat4 gVP;
uniform mat4 gWorld;
uniform vec4 vColor;
out vec4 Color;	
out vec2 TexCoord0;	

void main()                                                                   
{                                                                     
	 gl_Position =   gVP *  gWorld * vec4(vPosition, 1.0);
	 Color = vColor;
	 TexCoord0 = vTexCoord;
}
//---

