layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 textCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 VP;
uniform float currentCol;
uniform float currentRow;
uniform float frameWidth; 
uniform float frameHeight;

void main()
{
	gl_Position = VP * model * vec4(position, 1.0);
	TexCoord.x = (textCoord.x + currentCol) * frameWidth;
	TexCoord.y = (textCoord.y + currentRow) * frameHeight;
}
