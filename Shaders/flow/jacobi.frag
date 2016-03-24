//jacobi iteration fragment program

//output a color, which is the result quantity

#version 120

uniform float alpha,
uniform float rBeta; //reciprocal beta
uniform sampler2D x; //x vector (Ax = b)
uniform sampler2D b; //b vector (Ax = b)

void main()
{
	vec4 xL = texture2D( x, gl_FragCoord.xy - vec2( 1, 0 ) );
	vec4 xR = texture2D( x, gl_FragCoord.xy + vec2( 1, 0 ) );
	vec4 xB = texture2D( x, gl_FragCoord.xy - vec2( 0, 1 ) );
	vec4 xT = texture2D( x, gl_FragCoord.xy + vec2( 0, 1 ) );
	
	//b sample, from center
	vec4 bC = texture2D( b, gl_FragCoord.xy );
	
	
	gl_FragColor = (xL + xR + xB + xT + alpha * bC ) * rBeta;
}