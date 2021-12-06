#version 130

uniform sampler2D u_texture;

uniform vec4 u_quad1;
uniform vec4 u_quad2;

uniform vec4 skyColor;
uniform vec2 topLeft;
uniform float zoom;

uniform vec2 Resolution;      //resolution of screen
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 

uniform float u_slide;
uniform vec4 u_waterBaseColor;

uniform float u_cameraAngle;

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

void main()
{	
	float size = 128.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );	
	
	vec2 pos = topLeft + rotateUV( pixelPos, vec2( 0.0 ), u_cameraAngle );
	
	ivec2 texSize = textureSize(u_texture, 0);
	
	vec2 tileLimit = vec2( size ) / texSize;
	
	pos = mod( pos, size ) / texSize;
	//vec2 UV = mod( topLeft + pixelPos, size) / texSize;
	
	
	
	
	
	
	
	vec4 DiffuseColor;
	
	vec4 q1 = u_quad1;
	
	q1.x = q1.x + .001;
	q1.y = q1.y + .001;
	
	vec4 q2 = u_quad2;
	q2.x = q2.x + .001;
	q2.y = q2.y + .001;
	
	tileLimit.x = tileLimit.x - .001;
	tileLimit.y = tileLimit.y - .001;
	
	vec2 stuff0 = mod( pos + vec2( 0, u_slide * .2 ) * tileLimit, tileLimit ) + q1.xy;
	vec2 stuff1 = mod( pos + vec2( 0, u_slide * -.4 ) * tileLimit, tileLimit ) + q2.xy;
	vec2 stuff2 = mod( pos + vec2( u_slide * .2 + .2, 0 ) * tileLimit, tileLimit ) + q1.xy;
	vec2 stuff3 = mod( pos + vec2( u_slide * -.4 + .2, 0 ) * tileLimit, tileLimit ) + q2.xy;
	
	vec4 colora0 = texture2D( u_texture, stuff0.xy );
	vec4 colora1 = texture2D( u_texture, stuff1.xy );
	vec4 colora2 = texture2D( u_texture, stuff2.xy );
	vec4 colora3 = texture2D( u_texture, stuff3.xy );
	
	DiffuseColor = colora0;
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = colora1;
	}
	
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = colora2;
	}
	
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = colora3;
	}
	
	DiffuseColor.a = DiffuseColor.a * .5;
	
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = u_waterBaseColor;
	}
	
	gl_FragColor = gl_Color * DiffuseColor;
}

