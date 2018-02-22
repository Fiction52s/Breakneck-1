#version 120

void main()
{	
	vec4 col = vec4( 1.0 - gl_Color.r, 1-gl_Color.g, 1-gl_Color.b, gl_Color.a );
	gl_FragColor = col;
	
}

