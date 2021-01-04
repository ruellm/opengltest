out vec4 FragColor;

in vec2 TexCoord; // the input variable from the vertex shader(same name same type)
uniform sampler2D texture;
uniform float alpha;

void main()
{
	FragColor = texture(texture, TexCoord);
	// temporary because of the image
	if( FragColor.a <= 0.059)
		discard;
		
	//FragColor.a *= alpha;
}