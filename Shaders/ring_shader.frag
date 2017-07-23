#version 120

uniform sampler2D u_ringTex; 
uniform float u_startAngle;
uniform float u_filledRange;
uniform vec4 u_activeColor;
uniform vec4 u_removeColor;
uniform vec4 u_emptyColor;

#define PI 3.1415926535897932384626433832795

void main()
{
	vec2 tex = gl_TexCoord[0].xy;
	vec2 center = vec2( .5, .5 );
	vec2 diff = normalize( tex - center );
	vec4 texColor = texture2D(u_ringTex,  gl_TexCoord[0].xy );
	
	float angle = atan(diff.y, diff.x );
	float blahDiff = (u_startAngle+u_filledRange) - (PI - u_startAngle);
	float blahStart = (-PI - u_startAngle ) + blahDiff;
	vec2 other = vec2( sin( u_startAngle ), cos( u_startAngle ) );
	if( texColor.a == 1.0
		&& (
		( angle > u_startAngle && angle < u_startAngle + u_filledRange ) 
	 || ( angle < u_startAngle && u_startAngle + u_filledRange > PI && angle < blahStart ) ) )
	{
		gl_FragColor = u_activeColor;
	}
	else
	{
		gl_FragColor = texColor;
	}
	
}