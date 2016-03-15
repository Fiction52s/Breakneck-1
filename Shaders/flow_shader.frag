#version 120

uniform sampler2D u_texture;
uniform sampler2D u_normals;   //normal map
uniform sampler2D u_pattern;

uniform vec2 topLeft;
uniform float zoom;

//this makes things weird sometimes. need y coords reversed in a way.

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen

uniform float radius;
uniform float radDiff;
uniform vec2 goalPos;
uniform float flowSpacing;
uniform float maxFlowRings;



void main()
{	
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	
	vec2 pos = topLeft + pixelPos;
	
	float realLen = length( pos - goalPos );
	float len = mod( realLen , maxFlowRings );
	vec4 finalfinal = vec4( 1, 0, 0, 1 );
	
	if( (len < radius && len > radius - radDiff)
		|| (radius <= radDiff && len + (radDiff - radius) > maxFlowRings) )
	{
		finalfinal = vec4( 0, 0 , 1, 1 );
	}
	else
	{
		finalfinal = vec4( 0, 0, 0, 1 );
	}
	
	gl_FragColor =  finalfinal;//vec4(finalfinal, DiffuseColor.a);
	
	
}

