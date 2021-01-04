out vec4 FragColor;                                                           
in vec2 TexCoord0; 	
in vec4 Color;																			                                                      
uniform sampler2D gSampler;  
			
void main()                                                                   
{                                                                             
	FragColor =  texture(gSampler, TexCoord0) * Color;
	if (FragColor.a == 0)
		discard;
}
//---
