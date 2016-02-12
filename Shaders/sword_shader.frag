#version 120

uniform sampler2D u_texture;
uniform vec4 energyColor;

void main()
{	
	//vec2 fragC = gl_FragCoord.xy;
	//fragC.y = 1 - fragC.y;
	
	vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	
	if( DiffuseColor.rgb == vec3( 0, 0xee / 255.0, 0xff / 255.0 ) )
	{
		DiffuseColor = energyColor;//vec3( 0, 0x66 / 255.0, 0xcc / 255.0 );
	}
	gl_FragColor = DiffuseColor;
	
}

