layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vTexCoord;

layout (location = 2) in vec3 vPosition_2;
layout (location = 3) in vec4 vColor;
layout (location = 4) in float fSize;
layout (location = 5) in float rotationZ;

uniform mat4 gVP;

uniform int  renderAlign;
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
//uniform mat4 WorldMatrix;
uniform vec3 WorldPosition;
uniform vec2 stretchedScale;
uniform mat4 rotateMat;

out vec4 ParticleColor;	
out vec2 TexCoord;	

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()                                                                   
{           
	vec3 BillboardPos = vPosition_2;
	vec2 BillboardSize = vec2(fSize * stretchedScale.x, 
			fSize * stretchedScale.y);
	
	vec3 particleCenter_wordspace = WorldPosition + BillboardPos;
		
	if( renderAlign == 0 ) 
	{
		vec3 vertexPosition_worldspace = 
			particleCenter_wordspace
			+ CameraRight_worldspace * vPosition.x * BillboardSize.x
			+ CameraUp_worldspace * vPosition.y * BillboardSize.y;
			
		vec3 cameraLook = cross(CameraRight_worldspace, CameraUp_worldspace);
		mat4 rotate = rotationMatrix(cameraLook, rotationZ);

		// return to origin before rotate
		vertexPosition_worldspace -= particleCenter_wordspace;
		
		//apply rotation
		vec4 rotatedPosition = rotate * vec4(vertexPosition_worldspace, 1);
		vertexPosition_worldspace = rotatedPosition.xyz;
		
		// return to world position
		vertexPosition_worldspace += particleCenter_wordspace;
		
		gl_Position =   gVP * vec4(vertexPosition_worldspace, 1.0);
	} 
	else if (renderAlign == 1)
	{		
		vec3 transformedPosition = WorldPosition + (vPosition + vPosition_2) * vec3(BillboardSize,1);
		
		transformedPosition -= WorldPosition;
		
		mat4 rotate = rotationMatrix(vec3(0,0,1), rotationZ);
		vec4 rotatedPosition = rotateMat * rotate * vec4(transformedPosition, 1);
		transformedPosition = rotatedPosition.xyz;
		
		transformedPosition += WorldPosition;
		
		gl_Position =  gVP * (vec4(transformedPosition, 1.0));
	}

	ParticleColor = vColor;
	TexCoord = vTexCoord;
}

