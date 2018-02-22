//#version 120

uniform sampler2D u_texture;
uniform vec4 toColor;
uniform vec4 fromColor;

void main()
{		
	vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	
	if( DiffuseColor.a > 0.0 )
	{
		gl_FragColor = toColor;
	}
	else
	//if( DiffuseColor.rgb == fromColor.rgb )
	//{
	//	gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
		//DiffuseColor.rgb = toColor.rgb;
	//}
	
	gl_FragColor = DiffuseColor;
	//gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
	
}



