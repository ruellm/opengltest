layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTexCoord;
layout (location = 2) in vec3 vPosition_2;
layout (location = 3) in vec4 vColor;
layout (location = 4) in float fSize;

uniform mat4 gVP;
//uniform vec4 vColor;

//uniform vec3 BillboardPos; 
//uniform vec2 BillboardSize; 
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform mat4 WorldPos;

out vec4 Color;	
out vec2 TexCoord;	

void main()                                                                   
{                  

	vec3 BillboardPos = vPosition_2;
	vec2 BillboardSize = vec2(fSize, fSize);
	
	vec4 worldPos = WorldPos * vec4(BillboardPos, 1.0);
	vec3 particleCenter_wordspace = worldPos.xyz;
	
	vec3 vertexPosition_worldspace = 
		particleCenter_wordspace
		+ CameraRight_worldspace * vPosition.x * BillboardSize.x
		+ CameraUp_worldspace * vPosition.y * BillboardSize.y;
		
	 gl_Position =   gVP * vec4(vertexPosition_worldspace, 1.0);
	 Color = vColor;
	 TexCoord = vTexCoord;
}

