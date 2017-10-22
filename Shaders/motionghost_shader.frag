#version 120

uniform sampler2D u_texture;
uniform vec2 textureSize;
uniform vec4 energyColor;

void main()
{	

	vec2 tc = vec2( gl_TexCoord[0].x, gl_TexCoord[0].y );
	vec4 DiffuseColor = texture2D(u_texture, tc  );
	vec4 eColor = energyColor;
	
	float pix = tc.y * textureSize.y;
	if( DiffuseColor.a > 0 )
	{
		float f = mod( pix, 2 );
		DiffuseColor.rgb = eColor.rgb;
	}
	else
	{
		DiffuseColor.a = 0.0;
	}
	
	gl_FragColor = DiffuseColor;
	
}

