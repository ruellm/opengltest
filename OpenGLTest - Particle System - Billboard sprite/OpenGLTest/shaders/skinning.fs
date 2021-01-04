out vec4 FragColor;                                                           

in vec2 TexCoord0;
in vec3 Normal0;                                                                   
in vec3 WorldPos0;  
uniform sampler2D gSampler;                                                        

			
void main()                                                                   
{                                                                             
	//vec4 lightColor = GetCombinedLightColor(Normal0);
	vec4 localColor = texture(gSampler, TexCoord0.xy);   
	FragColor =  localColor;// * lightColor;
}
//---
