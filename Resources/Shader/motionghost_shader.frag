#version 120

uniform sampler2D u_texture;
//uniform vec4 energyColor;

void main()
{	

	vec2 tc = vec2( gl_TexCoord[0].x, gl_TexCoord[0].y );
	vec4 DiffuseColor = texture2D(u_texture, tc  );
	//vec4 eColor = energyColor;
	
	
	if( DiffuseColor.a > 0 )
	{
		DiffuseColor = gl_Color;
		//DiffuseColor.rgb = eColor.rgb;
		//DiffuseColor.a = gl_Color.a;
	}
	else
	{
		DiffuseColor.a = 0.0;
	}
	
	gl_FragColor = DiffuseColor;
	
}

