uniform sampler2D u_noiseTex;
uniform sampler2D u_gradTex;

uniform float u_time;

void main()
{
	vec2 TexCoord = vec2(gl_TexCoord[0].xy);
	vec2 coord = TexCoord;
	vec2 noiseCoord = coord;
	noiseCoord.y = fract( noiseCoord.y + u_time );
	
	vec4 noise = texture2D(u_noiseTex, noiseCoord );
	vec4 grad = texture2D( u_gradTex, coord );
	
	noise.a = step( (1.0- .5 * grad.r), noise.r );
	
	vec4 col = noise;
	//col.a = noise.a;
	
	col.g *= 1.0 + grad.r + grad.r * noise.r;
	//vec4 col = noise;
	gl_FragColor = col;
}
