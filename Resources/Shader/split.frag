uniform sampler2D u_texture;
uniform sampler2D u_splitTexture;
uniform vec2 offset;
uniform float currAlpha;

void main()
{
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 splitCoord = TexCoord + offset;
	vec4 col = texture2D(u_texture, TexCoord );
	vec4 colSplit = texture2D(u_splitTexture, splitCoord);
	
	col.a = col.a * colSplit.a * currAlpha;
	gl_FragColor = col;
}

