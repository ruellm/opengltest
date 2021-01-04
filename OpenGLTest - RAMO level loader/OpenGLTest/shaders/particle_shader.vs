layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTexCoord;

uniform mat4 gVP;
uniform vec4 vColor;

uniform vec3 BillboardPos; // Position of the center of the billboard
uniform vec2 BillboardSize; // Size of the billboard, in world units (probably meters)
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;

out vec4 Color;	
out vec2 TexCoord0;	

void main()                                                                   
{                      
	vec3 particleCenter_wordspace = BillboardPos;
	
	vec3 vertexPosition_worldspace = 
		particleCenter_wordspace
		+ CameraRight_worldspace * vPosition.x * BillboardSize.x
		+ CameraUp_worldspace * vPosition.y * BillboardSize.y;
		
	 gl_Position =   gVP * vec4(vertexPosition_worldspace, 1.0);
	 Color = vColor;
	 TexCoord0 = vTexCoord;
}
//---

