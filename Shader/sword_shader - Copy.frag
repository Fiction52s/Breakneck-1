#version 120

uniform sampler2D u_texture;
uniform float onPortion; 

void main()
{	
	vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	DiffuseColor.a = 1.0 * ( onPortion > gl_FragCoord.x );
	//vec2 fragC = gl_FragCoord.xy;
	//fragC.y = 1 - fragC.y;
	
	
	vec4 eColor = energyColor;
	if( isTealAlready == 0 ) //not teal already mode
	{
		if( DiffuseColor.a > 0 )
		{
			eColor.r = 34.0 / 255;
			eColor.g = 119.0 / 255;
			eColor.b = 51.0 / 255;
			eColor.a = opacity;
			DiffuseColor = eColor;
		}
		
	}
	else //already teal mode (the sword)
	{
		if( DiffuseColor.rgb == vec3( 0, 0xee / 255.0, 0xff / 255.0 ) )
		{
			DiffuseColor = eColor;//vec3( 0, 0x66 / 255.0, 0xcc / 255.0 );
		}
	}
	
	
	gl_FragColor = DiffuseColor;
	
}

