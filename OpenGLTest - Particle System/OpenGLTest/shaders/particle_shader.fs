out vec4 FragColor;                                                           
in vec4 Color; 																			                                                      
uniform sampler2D gSampler;  
			
void main()                                                                   
{                                                                             
	FragColor =  texture(gSampler, gl_PointCoord) * Color;
	if (FragColor.a == 0)
		discard;
}
//---
