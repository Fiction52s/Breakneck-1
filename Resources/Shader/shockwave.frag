//#define COLOR_TEAL Color( 0, 0xee, 0xff )
//#define COLOR_BLUE Color( 0, 0x66, 0xcc )
//#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
//#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
//#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
//#define COLOR_RED Color( 0xff, 0x22, 0 )
//#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
//#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


#version 120

uniform sampler2D shockwaveTex;
uniform sampler2D underTex;
uniform vec2 texSize;
uniform vec2 resolution;
uniform vec2 botLeft;
uniform float zoom;

const vec3 TEAL = vec3( 0.0, 0xee / 255.0, 0xff / 255.0 );
const vec3 BLUE = vec3( 0.0, 0x66 / 255.0, 0xcc / 255.0 );

void main()
{
	//vec4 displacement = texture2D(shockwaveTex, gl_FragCoord.xy / texSize );
	
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 960.0, 540.0 ) / resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	//vec2 pos = botLeft + pixelPos;
	
	
	
	vec4 shockColor = texture2D(shockwaveTex, gl_TexCoord[0].xy );
	vec2 pos = gl_FragCoord.xy;
	float fac = 100.0;//.025;
	
	vec2 offset = vec2( shockColor.r * 2.0 - 1.0, shockColor.g * 2.0 - 1.0 );
	//offset = vec2( 0, 1);
    pos.x += offset.x * fac;
    pos.y -= offset.y * fac;
	
	vec4 underColor = texture2D(underTex, pos.xy / resolution );
	
	underColor.a = shockColor.a;
	gl_FragColor = underColor;
}