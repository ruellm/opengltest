out vec4 FragColor;
                                                   
in vec2 TexCoord; 	
in vec4 ParticleColor;			
																                                                      
uniform sampler2D gSampler;  
uniform float alpha;
uniform int isReplaceColor;

void main()                                                                   
{       

	if( isReplaceColor == 1 ) 
	{                                                                      	
		vec4    color   = texture(gSampler, TexCoord.xy);
		FragColor    = color * ParticleColor;
	}
	else {
		vec4 localcolor =  texture(gSampler, TexCoord);
		if (localcolor.a == 0.0)
			discard;
	
		FragColor = localcolor;	
	}
	
	FragColor.a = FragColor.a * alpha;	
}
