in vec3 position; 
in vec2 textCoord;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 VP;
uniform float currentFrame;
uniform float frameWidth; 

void main()
{
	gl_Position = VP * model * vec4(position, 1.0);
	TexCoords.x = (textCoord.x + currentFrame) * frameWidth;
	TexCoords.y = textCoord.y;
}
