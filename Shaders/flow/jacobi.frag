//jacobi iteration fragment program

//output a color, which is the result quantity

#version 120

uniform float alpha;
uniform float rBeta; //reciprocal beta
uniform sampler2D x; //x vector (Ax = b)
uniform sampler2D b; //b vector (Ax = b)
uniform vec2 texSize;

void main()
{
	vec4 xL = texture2D( x, (gl_FragCoord.xy - vec2( 1, 0 )) / texSize );
	vec4 xR = texture2D( x, (gl_FragCoord.xy + vec2( 1, 0 )) / texSize);
	vec4 xB = texture2D( x, (gl_FragCoord.xy - vec2( 0, 1 )) / texSize );
	vec4 xT = texture2D( x, (gl_FragCoord.xy + vec2( 0, 1 )) / texSize );
	
	//b sample, from center
	vec4 bC = texture2D( b, gl_FragCoord.xy / texSize );
	bC = -bC;
	
	gl_FragColor = (bC *alpha + xL + xR + xB + xT ) * rBeta;
	
	
	
	
	
	//(bC + alpha * ( xL + xR + xB + xT )) / rBeta;
	//gl_FragColor = //(bC + alpha * ( xL + xR + xB + xT )) * rBeta;
	//gl_FragColor = bC * alpha + (xL + xR + xB + xT ) *rBeta;//(xL + xR + xB + xT + 1 * bC ) * .1;
	
}