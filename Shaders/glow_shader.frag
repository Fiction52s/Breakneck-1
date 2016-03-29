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
uniform sampler2D old;
uniform vec2 texSize;

const vec4 TEAL = vec4( 0, 0xee / 255.0, 0xff / 255.0, 1 );

bool IsGlowing( vec4 col )
{
	return col.rgb == TEAL.rgb;
}

void main()
{
	vec4 col = texture2D(tex, gl_FragCoord.xy / texSize );
	gl_FragColor = vec4( 0, 0, 0, 1 ) + float(IsGlowing( col ) ) * col;
}