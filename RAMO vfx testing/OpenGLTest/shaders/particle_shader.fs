out vec4 FragColor;
                                                   
in vec2 TexCoord; 	
in vec4 ParticleColor;			
																                                                      
uniform sampler2D gSampler;  
uniform float alpha;

void main()                                                                   
{                                                                             	
	const vec4  kRGBToYPrime = vec4 (0.299, 0.587, 0.114, 0.0);
    const vec4  kRGBToI     = vec4 (0.596, -0.275, -0.321, 0.0);
    const vec4  kRGBToQ     = vec4 (0.212, -0.523, 0.311, 0.0);

    const vec4  kYIQToR   = vec4 (1.0, 0.956, 0.621, 0.0);
    const vec4  kYIQToG   = vec4 (1.0, -0.272, -0.647, 0.0);
    const vec4  kYIQToB   = vec4 (1.0, -1.107, 1.704, 0.0);

    // Sample the input pixel
    vec4    color   = texture(gSampler, TexCoord.xy);
	vec4	color2  = ParticleColor;
	
    // Convert to YIQ
    float   YPrime  = dot (color, kRGBToYPrime);
    float   I      	= dot (color, kRGBToI);
    float   Q      	= dot (color, kRGBToQ);
	
	 // Convert REPLACEMENT COLOR to YIQ
    float   YPrime2  = dot (color2, kRGBToYPrime);
    float   I2      = dot (color2, kRGBToI);
    float   Q2      = dot (color2, kRGBToQ);

    // Calculate the hue and chroma for REPLACEMENT color
    float   hue2     = atan (Q2, I2);
    float   chroma2  = sqrt (I2 * I2 + Q2 * Q2);

    // Convert back to YIQ
    Q = chroma2 * sin (hue2);
    I = chroma2 * cos (hue2);

    // Convert back to RGB
    vec4    yIQ   = vec4 (YPrime, I, Q, 0.0);
    color.r = dot (yIQ, kYIQToR);
    color.g = dot (yIQ, kYIQToG);
    color.b = dot (yIQ, kYIQToB);

    // Save the result
    FragColor    = color * ParticleColor;
}