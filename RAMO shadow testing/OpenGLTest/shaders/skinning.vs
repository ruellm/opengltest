in vec3 Position;                                             
in vec2 vTexCoord;                                             

in vec3 vNormal;                                               
//in ivec4 BoneIDs;
in vec4 BoneIDs;
in vec4 Weights;

out vec2 TexCoord;
out vec3 Normal;                                                                   
out vec3 WorldPos;                                                                 

out vec4 FragPosLightSpace;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 worldMatrix;

uniform mat4 lightSpaceMatrix;

uniform mat4 gBones[50];
uniform int  withBones;

void main()
{       
	mat4 gWVP = projectionMatrix * viewMatrix * worldMatrix;
	
	if(withBones == 1) {
	
		highp int bid0 = int(BoneIDs[0]);
		highp int bid1 = int(BoneIDs[1]);
		highp int bid2 = int(BoneIDs[2]);
		highp int bid3 = int(BoneIDs[3]);
		
		mat4 BoneTransform = gBones[bid0] * Weights[0];
		BoneTransform     += gBones[bid1] * Weights[1];
		BoneTransform     += gBones[bid2] * Weights[2];
		BoneTransform     += gBones[bid3] * Weights[3];

        vec4 PosL    = BoneTransform * vec4(Position, 1.0);
        gl_Position  = gWVP * PosL;
        vec4 NormalL = BoneTransform * vec4(vNormal, 0.0);
        Normal      = (worldMatrix * NormalL).xyz;
        WorldPos    = (worldMatrix * PosL).xyz;
		
		FragPosLightSpace = lightSpaceMatrix * worldMatrix * PosL;
		
	} else {
	    gl_Position  = gWVP * vec4(Position, 1.0);

        Normal      = (worldMatrix * vec4(vNormal, 0.0)).xyz;
        WorldPos    = (worldMatrix * vec4(Position, 1.0)).xyz;
		
		FragPosLightSpace = lightSpaceMatrix * worldMatrix * vec4(Position, 1.0);
	}

    TexCoord    = vTexCoord;

}
