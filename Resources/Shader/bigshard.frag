uniform float shine;
uniform sampler2D u_texture;

void main()
{		
	vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	float fac = 1.0 / 5.0; //based on number of tiles
	vec2 relCoord = mod( gl_TexCoord[0], .2 );
	relCoord = relCoord * vec2( 5.0, 5.0 );
	
	float diff = abs( shine - relCoord.x );
	DiffuseColor.a = 1.0 - diff;
	
	gl_FragColor = DiffuseColor;
}

