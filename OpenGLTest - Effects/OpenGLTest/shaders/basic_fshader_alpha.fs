out vec4 FragColor;

in vec2 TexCoord; // the input variable from the vertex shader(same name same type)
uniform sampler2D texture;
uniform float alpha;

void main()
{
	FragColor = texture(texture, TexCoord) * vec4(0,1,1,1);	
	FragColor.a *= alpha;
}