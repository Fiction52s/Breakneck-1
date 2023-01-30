#version 130

uniform sampler2D u_texture;
uniform sampler2D u_pattern;

uniform vec4 skyColor;
uniform vec2 topLeft;
uniform float zoom;
uniform vec4 u_quadArray[4];

uniform float u_cameraAngle;
uniform float u_patternGrid[16];

float patternGridSize = 4.0;

uniform vec2 Resolution;      //resolution of screen
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 

vec2 rotateUV(in vec2 uv, in vec2 pivot, in float rotation) {
    float sine = sin(rotation);
    float cosine = cos(rotation);

    uv -= pivot;
	float tempX = uv.x * cosine - uv.y * sine;
    uv.y = uv.x * sine + uv.y * cosine;
	uv.x = tempX;
    uv += pivot;

    return uv;
}

float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio   

float gold_noise(vec2 xy, float seed){
       return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);}


void main()
{	
	float size = 256.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );

	vec2 pos = topLeft + rotateUV( pixelPos, vec2( 0.0 ), u_cameraAngle );
	
	ivec2 texSize = textureSize(u_texture, 0);
	
	vec2 tileLimit = vec2( size ) / texSize;
	
	vec4 finalfinal = vec4( 0.0, 0.0, 0.0, 0.0 );
	
	vec4 DiffuseColor;
	
	vec2 test = pos / texSize;
	
	
	vec2 UV = vec2( 0.0, 0.0 );
	
	
	int xTest = int(mod( floor(test.x / tileLimit.x), patternGridSize ));
	int yTest = int(mod( floor(test.y / tileLimit.y), patternGridSize ));
	
	int index = yTest * int(patternGridSize) + xTest;
	
	int tileBlah = int(u_patternGrid[index]);
	
	UV = mod( test, tileLimit) + u_quadArray[tileBlah].xy;
	
	DiffuseColor = texture2D(u_texture, UV );
	
	//vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
	//vec3 Intensity = Ambient;//.000001;
	//finalfinal = vec4( DiffuseColor.rgb * Intensity, DiffuseColor.a );
	
	finalfinal = DiffuseColor;
	
	finalfinal = gl_Color * finalfinal;
	
	finalfinal.rgb = vec3(.9) * finalfinal.rgb + vec3( .1 ) * skyColor.rgb;
	
	gl_FragColor = finalfinal;
}

