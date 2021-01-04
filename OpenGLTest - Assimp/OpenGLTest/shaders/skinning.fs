out vec4 FragColor;                                                           

in vec2 TexCoord;
in vec3 Normal;                                                                   
in vec3 WorldPos;  
uniform sampler2D gSampler;                                                        

			
void main()                                                                   
{                                                                             
	//vec4 lightColor = GetCombinedLightColor(Normal0);
	vec4 localColor = texture(gSampler, TexCoord.xy);   
	FragColor =  localColor;// * lightColor;
}
//---
