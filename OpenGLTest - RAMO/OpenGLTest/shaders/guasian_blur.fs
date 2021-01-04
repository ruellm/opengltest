out vec4 FragColor;

in vec2 blurTextureCoords[11];
uniform sampler2D image;

void main()
{    
	// http://dev.theomader.com/gaussian-kernel-calculator/
	FragColor = vec4(0.0);
	FragColor += texture(image, blurTextureCoords[0]) * 0.0093;
    FragColor += texture(image, blurTextureCoords[1]) * 0.028002;
    FragColor += texture(image, blurTextureCoords[2]) * 0.065984;
    FragColor += texture(image, blurTextureCoords[3]) * 0.121703;
    FragColor += texture(image, blurTextureCoords[4]) * 0.175713;
    FragColor += texture(image, blurTextureCoords[5]) * 0.198596;
    FragColor += texture(image, blurTextureCoords[6]) * 0.175713;
    FragColor += texture(image, blurTextureCoords[7]) * 0.121703;
    FragColor += texture(image, blurTextureCoords[8]) * 0.065984;
    FragColor += texture(image, blurTextureCoords[9]) * 0.028002;
    FragColor += texture(image, blurTextureCoords[10]) * 0.0093;
}