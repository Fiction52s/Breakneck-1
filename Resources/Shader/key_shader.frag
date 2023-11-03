#version 120

uniform sampler2D u_texture;
uniform float prop;
uniform vec4 toColor;
uniform vec4 auraColor;
//uniform vec4 fromColor;

void main()
{		
	vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	
	if( DiffuseColor != auraColor )
	{
		DiffuseColor.rgb = DiffuseColor.rgb * vec3( 1.0 - prop ) + toColor.rgb * vec3(prop);
	}
	
	gl_FragColor = DiffuseColor;
	
}

