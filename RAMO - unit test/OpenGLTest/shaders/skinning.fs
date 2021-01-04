out vec4 FragColor;                                                           

in vec2 TexCoord;
in vec3 Normal;                                                                   
in vec3 WorldPos;  
uniform sampler2D gSampler;                                                        
			
void main()                                                                   
{                                                                             
	vec4 localColor = texture(gSampler, TexCoord.xy);   
	
	vec4 finalColor = localColor;
	vec4 diffuseColor  = vec4(1, 1, 1, 1);

	FragColor =  finalColor;
}
//---