#version 120

uniform sampler2D u_texture;
uniform sampler2D u_normals;   //normal map
uniform sampler2D u_pattern;

uniform vec4 flowColor;
uniform vec4 closeFlowColor;

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

uniform float u_cameraAngle;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 rotateUV(in vec2 uv, in vec2 pivot, in float rotation) {
    float sine = sin(rotation);
    float cosine = cos(rotation);

    uv -= pivot;
	float tempX = uv.x * cosine - uv.y * sine;
    uv.y = uv.x * sine + uv.y * cosine;
	uv.x = tempX;
    uv += pivot;

    return uv;
}

void main()
{	
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	
	vec2 pos = topLeft + rotateUV( pixelPos, vec2( 0.0 ), u_cameraAngle );
	
	//vec2 pos = topLeft + pixelPos;
	
	float realLen = length( pos - goalPos );
	float len = mod( realLen , maxFlowRings );
	float lenPlayer = length( pos - playerPos );
	float playerToGoal = length( playerPos - goalPos );
	vec4 finalfinal = vec4( 1, 0, 0, 1 );
	
	bool res = (len < radius && len > radius - radDiff)
		|| (radius <= radDiff && len + (radDiff - radius) > maxFlowRings);
	float fres = float(res);
	
	float alpha = rand( vec2( pos.x + radius, pos.y + radius ) );
	
	finalfinal = vec4( flowColor.r * fres, flowColor.g * fres , flowColor.b * fres, alpha * .7 );
	
	if( !res )
		finalfinal = vec4( 0, 0, 0, 1 );
	
	float d = dot( normalize( goalPos - playerPos ), normalize( goalPos - pos ));
	
	if( res && realLen <= playerToGoal - 40 )//&& d > .99 )
	{
		float f = 1 - ((1 - d) / .01);
		float sameDirection = 0.999;
		float sortOfDirection = 0.97;
		
		if( d > sameDirection )
		{
			finalfinal = closeFlowColor;
		}
		else if ( d > sortOfDirection )
		{
			float test = 1.0 - (sameDirection - d) / (sameDirection - sortOfDirection );
			finalfinal = mix( flowColor, closeFlowColor, test);
		}
		
		//finalfinal = mix( flowColor, closeFlowColor, d * 0.5 );
		finalfinal.a = alpha * 5.0;
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

