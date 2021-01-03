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


void main()
{	
	float size = 128.0;//512.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );	
	
	ivec2 texSize = textureSize(u_texture, 0);
	vec2 pos = mod( topLeft + pixelPos, size) / texSize;
	
	//pos = vec2(clamp( pos.x, u_quad1.x + .0001, u_quad1.z - .0001 ), 
	//clamp( pos.y, u_quad1.y + .0001, u_quad1.w - .0001 ) );
	
	vec2 quadSize = u_quad1.zw - u_quad1.xy;
	vec2 stuff = mod(pos - u_quad1.xy, quadSize);// / quadSize.xy;
	//pos = stuff;
	
	vec2 tileLimit = vec2( size ) / texSize;
	
	vec4 DiffuseColor;
	
	vec2 stuff0 = mod( pos + vec2( 0, u_slide * .1 ) * tileLimit, tileLimit ) + u_quad1.xy;
	vec2 stuff1 = mod( pos + vec2( 0, u_slide * -.2 ) * tileLimit, tileLimit ) + u_quad2.xy;
	vec2 stuff2 = mod( pos + vec2( u_slide * .1 + .1, 0 ) * tileLimit, tileLimit ) + u_quad1.xy;
	vec2 stuff3 = mod( pos + vec2( u_slide * -.2 + .1, 0 ) * tileLimit, tileLimit ) + u_quad2.xy;
	
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
	
	DiffuseColor = DiffuseColor * .5;
	
	if( DiffuseColor.a == 0 )
	{
		DiffuseColor = u_waterBaseColor;
	}
	
	gl_FragColor = gl_Color * DiffuseColor;
}

