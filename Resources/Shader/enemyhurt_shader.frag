#version 120

uniform sampler2D u_texture;
uniform vec4 toColor;
uniform vec4 auraColor;
//uniform vec4 fromColor;

void main()
{		
	vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	
	//if( DiffuseColor.rgb != auraColor.rgb )
	DiffuseColor.rgb = toColor.rgb * .9 + DiffuseColor.rgb * .1;
	
	
	gl_FragColor = DiffuseColor;
	
}

