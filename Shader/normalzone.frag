//uniform sampler2D u_texture;
uniform vec4 shadowColor;
uniform float alpha;


void main()
{		
	vec4 DiffuseColor = shadowColor;//texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y ) );
	
	DiffuseColor.a = DiffuseColor.a * alpha;
	
	gl_FragColor = DiffuseColor;
	
}

