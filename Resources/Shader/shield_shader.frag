#version 120

uniform sampler2D u_texture;
uniform vec4 toColor;

void main()
{		
	vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	
	DiffuseColor.rgb = toColor.rgb;
	
	gl_FragColor = DiffuseColor;
	
}