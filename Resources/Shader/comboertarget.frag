#version 130

uniform sampler2D u_texture;
uniform vec4 u_palette[18];
//uniform int u_hurt; 

void main()
{		
	vec4 texColor = texture2D(u_texture, gl_TexCoord[0].xy );
	
	int texIndex = int(texColor.r * 255.0);
	vec4 DiffuseColor = vec4( u_palette[texIndex].rgb, texColor.a);
	
	//if( u_hurt == 1.0 )
	//{
	//	DiffuseColor.rgb = toColor.rgb * .9 + DiffuseColor.rgb * .1;
	//}
	
	
	gl_FragColor = DiffuseColor * gl_Color;
	
}

