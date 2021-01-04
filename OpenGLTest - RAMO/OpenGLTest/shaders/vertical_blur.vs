layout (location = 0) in vec3 position; // the position variable has attribute position 0
layout (location = 1) in vec2 textCoord;

out vec2 blurTextureCoords[11];
uniform float targetWidth;

void main()
{
	gl_Position = vec4(position, 1.0);
	
	float pixelSize = 1.0 / targetWidth;
	for(int i=-5; i <=5;i++)
	{
		blurTextureCoords[i+5] = textCoord + vec2(0, pixelSize * float(i));
	}
}
