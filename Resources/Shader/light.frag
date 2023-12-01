#version 120

uniform float zoom;
//uniform vec2 topLeft;
uniform vec2 resolution;
uniform vec2 pos;
uniform vec3 lightpos;
uniform vec3 falloff;
//layout(origin_upper_left) in vec4 gl_FragCoord;

void main()
{
	vec4 yo = vec4( 1, 0, 0, 1 );
//	float len = length( pos + gl_FragCoord.xy
	//float quant = 100 / length( pos );//+ gl_FragCoord.xy - lightpos );
	//yo.a = quant;
	
	vec2 fragC = gl_FragCoord.xy;
	fragC.y = 1 - fragC.y;
	vec3 LightDir = vec3(lightpos.xy - (vec2( fragC.x, fragC.y) / resolution.xy), lightpos.z);
	
	LightDir.x *= resolution.x / resolution.y;
	
	float D = length(LightDir) * zoom;		
	
	float Attenuation = 1.0 / ( falloff.x + (falloff.y*D) + (falloff.z*D*D) );
	
	gl_FragColor = gl_Color * Attenuation;
	
}

