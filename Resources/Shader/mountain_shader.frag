#version 120

uniform sampler2D u_texture;
uniform vec2 Resolution;
uniform float zoom;
uniform vec2 topLeft;

void main()
{	
	//float size = 1920.0;
	vec2 size = vec2( 1920.0, 1024.0 );
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 960.0, 540.0 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	
	vec2 pos;
	
	
	if( mod( floor( abs(topLeft.x + pixelPos.x) / 960.0 ), 2.0 ) == 0.0 )
	{
		pos = mod( topLeft + pixelPos + vec2( 0.0, -(size.y - .5) ), size) / vec2( size.x, size.y );
		//pos.y = pos.y - .5;
		pos.y = max( pos.y, .49 );
		
	}
	else
	{
		pos = mod( topLeft + pixelPos, size + vec2( 0.0, .5 ) ) / vec2( size.x, size.y );
		pos.y = max( pos.y, .49 );
	}
	
	vec4 DiffuseColor = texture2D(u_texture, pos);
	
	gl_FragColor = DiffuseColor;
	
	
	
	//gl_FragColor = col;
	
}