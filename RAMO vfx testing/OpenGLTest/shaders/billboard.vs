layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTexCoord;

uniform mat4 gVP;
uniform vec4 vColor;

uniform vec3 BillboardPos; // Position of the center of the billboard
uniform vec2 BillboardSize; // Size of the billboard, in world units (probably meters)
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform mat4 WorldPos;

uniform float currentFrame;
uniform float frameWidth; 

out vec4 Color;	
out vec2 TexCoord;	

void main()                                                                   
{                      
	vec4 worldPos = WorldPos * vec4(BillboardPos, 1.0);
	vec3 particleCenter_wordspace = worldPos.xyz;
	
	vec3 vertexPosition_worldspace = 
		particleCenter_wordspace
		+ CameraRight_worldspace * vPosition.x * BillboardSize.x
		+ CameraUp_worldspace * vPosition.y * BillboardSize.y;
		
	gl_Position =   gVP * vec4(vertexPosition_worldspace, 1.0);
	Color = vColor;
	TexCoord.x = (vTexCoord.x + currentFrame) * frameWidth;
	TexCoord.y = vTexCoord.y;
}

