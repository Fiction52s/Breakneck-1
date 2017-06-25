uniform sampler2D texture;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
	
	//pixel.r = 1.0;
	if( pixel.r > .8 && pixel.g > .8 && pixel.b > .8 )
	{
	pixel.a = 0.0;
	}
	
    // multiply it by the color
    gl_FragColor = gl_Color * pixel;
}