layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 textCoord;

out vec2 TexCoord; //specify a color output to the fragment shader
uniform mat4 model;
uniform mat4 VP;
uniform int currentFrame;
uniform float frameWidth; 

void main()
{
	gl_Position = VP * model * vec4(position, 1.0);
	TexCoord.x = (textCoord.x + currentFrame) * frameWidth;
	TexCoord.y = textCoord.y;
}
