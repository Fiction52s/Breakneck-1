#version 120

uniform sampler2D u_texture;
uniform vec2 topLeft;
uniform vec2 size;

void main()
{
	vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	
	
}

