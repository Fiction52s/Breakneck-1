#version 130

uniform sampler2D u_texture;
uniform vec4 u_palette[16];

void main()
{		
	vec4 texColor = texture2D(u_texture, gl_TexCoord[0].xy );
	
	int texIndex = int(texColor.r * 255.0);
	vec4 DiffuseColor = vec4( u_palette[texIndex].rgb, texColor.a);
	
	gl_FragColor = DiffuseColor * gl_Color;
	
}

