uniform sampler2D barTex;
uniform float tile;
uniform float factor;

void main()
{
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 trueCoord = vec2( ( TexCoord.x - (1.0/3.0 * tile )) * 3.0, TexCoord.y );
	float f = factor;
	vec2 test = vec2( f , 1.0 - f );
	vec2 dir = normalize( vec2( 1.0, -1.0 ) );
	vec2 testDir = normalize( trueCoord - test );
	
	float d = dot( testDir, dir );
	
	if( d > 0.0 )
	{
		//gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
		discard;
	}
	
	gl_FragColor = texture2D(barTex, TexCoord );
}

