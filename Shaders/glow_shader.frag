//#define COLOR_TEAL Color( 0, 0xee, 0xff )
//#define COLOR_BLUE Color( 0, 0x66, 0xcc )
//#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
//#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
//#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
//#define COLOR_RED Color( 0xff, 0x22, 0 )
//#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
//#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


#version 120

uniform sampler2D tex;
uniform sampler2D texSize;

void main()
{
	vec4 col = texture2D(tex, gl_FragCoord.xy / texSize );
	vec4 teal = vec4( 0, 0xee / 256.0, 0xff / 256.0, 1 );
	
	if( col != teal )
	{
		gl_FragColor = vec4( 0, 0, 0, 1 );
	}
	
}