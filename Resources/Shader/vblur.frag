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

uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
//uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

uniform float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );




void main()
{
	gl_FragColor = texture2D(tex, gl_FragCoord.xy / texSize ) * weight[0];
	for( int i = 1; i <= 4; i++ )
	{
	
		gl_FragColor += texture2D( tex, (gl_FragCoord.xy - vec2( 0, i )) / texSize ) * weight[i];
		gl_FragColor += texture2D( tex, (gl_FragCoord.xy + vec2( 0, i )) / texSize ) * weight[i];
	}
}