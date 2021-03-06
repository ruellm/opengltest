layout (location = 0) in vec3 Position;                                             
layout (location = 1) in vec2 vTexCoord;                                             
layout (location = 2) in vec3 vNormal;                                               
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 Weights;

out vec2 TexCoord;
out vec3 Normal;                                                                   
out vec3 WorldPos;                                                                 

const int MAX_BONES = 50;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 worldMatrix;

uniform mat4 gBones[MAX_BONES];
uniform int  withBones = 0;

void main()
{       
	mat4 gWVP = projectionMatrix * viewMatrix * worldMatrix;
	if(withBones == 1) {
		mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
		BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
		BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
		BoneTransform     += gBones[BoneIDs[3]] * Weights[3];

		vec4 PosL    = BoneTransform * vec4(Position, 1.0);
		gl_Position  = (gWVP) * PosL;
		
		vec4 NormalL = BoneTransform * vec4(vNormal, 0.0);
		Normal      = (worldMatrix * NormalL).xyz;
		WorldPos    = (worldMatrix * PosL).xyz;   
		
	} else {
		gl_Position  = gWVP * vec4(Position, 1.0);
		
		Normal      = (worldMatrix * vec4(vNormal, 0.0)).xyz;
		WorldPos   = (worldMatrix * vec4(Position, 1.0)).xyz;   
	}
	
	TexCoord    = vTexCoord;
	
	if(fog_vs_enable == 1)
	 {
		vec4 worldPosition = worldMatrix * vec4(Position, 1.0f);
		vec4 positionRelativeToCam = viewMatrix * worldPosition;
		Fog_ComputeVisibility(positionRelativeToCam);
	 }
	 
	 if(lightCountVS > 0)
	 {
		ComputeLightVector(WorldPos);
	 }
}
