out vec4 FragColor;

uniform vec4 color;
uniform float alpha;

void main()                                                                   
{                                                                             
	FragColor = color;		
	FragColor.a *= alpha;
}
