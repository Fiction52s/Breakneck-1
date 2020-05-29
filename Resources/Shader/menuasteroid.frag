uniform sampler2D u_texture;
uniform float quant;

void main()
{		
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 coord = TexCoord;
	
	coord.x = fract( TexCoord.x + quant );
	vec4 col = texture2D(u_texture, coord );
	gl_FragColor = col;
}

