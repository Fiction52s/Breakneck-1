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
uniform vec2 playerPos;


float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{	
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	
	vec2 pos = topLeft + pixelPos;
	
	float realLen = length( pos - goalPos );
	float len = mod( realLen , maxFlowRings );
	float lenPlayer = length( pos - playerPos );
	float playerToGoal = length( playerPos - goalPos );
	vec4 finalfinal = vec4( 1, 0, 0, 1 );
	
	bool res = (len < radius && len > radius - radDiff)
		|| (radius <= radDiff && len + (radDiff - radius) > maxFlowRings);
	float alpha = rand( vec2( pos.x + radius, pos.y + radius ) );
	//0, 0x66, 0xcc
	finalfinal = vec4( 0, 0x66 / 255.0 * float(res) , 0xcc / 255.0 * float(res), alpha * .9 );
	
	if( !res )
		finalfinal = vec4( 0, 0, 0, 1 );
	
	//0, 0xee, 0xff
	if( res && realLen <= playerToGoal && dot( normalize( goalPos - playerPos ), normalize( goalPos - pos )) > .99 )
	{
		finalfinal = vec4( 0, 0xee / 255.0, 0xff / 255.0, alpha * .9 );
	}
	
	//if( (len < radius && len > radius - radDiff)
	//	|| (radius <= radDiff && len + (radDiff - radius) > maxFlowRings) )
	//{
	//	finalfinal = vec4( 0, 0 , 1, 1 );
	//}
	//else
	//{
	//	finalfinal = vec4( 0, 0, 0, 1 );
	//}
	
	gl_FragColor =  finalfinal;//vec4(finalfinal, DiffuseColor.a);
	
	
}

