layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
                                                   
in vec2 TexCoord; 	
in vec4 Color;			
																                                                      
uniform sampler2D gSampler;  
uniform float alpha;
uniform int glow;

void main()                                                                   
{                                                                             
	vec4 localcolor =  texture(gSampler, TexCoord) * Color;
	if (localcolor.a == 0.0)
		discard;
	
	FragColor = localcolor;	
	FragColor.a = localcolor.a * alpha;	
	
	if(glow == 1)
		BrightColor = localcolor;
	else
		BrightColor = vec4(0,0,0,1);
}

