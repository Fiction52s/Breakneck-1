#version 130

uniform sampler2D u_texture;
uniform sampler2D u_pattern;

uniform vec4 skyColor;
uniform vec2 topLeft;
uniform float zoom;

uniform vec2 Resolution;      //resolution of screen
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 




void main()
{	
	float size = 512.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1.0 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );	
	
	vec2 pos = mod( topLeft + pixelPos, size) / vec2( size );
	
	vec4 finalfinal = vec4( 0.0, 0.0, 0.0, 0.0 );
	
	vec4 DiffuseColor;
	
	DiffuseColor = texture2D(u_texture, pos );
	
	vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
	vec3 Intensity = Ambient;//.000001;
	finalfinal = vec4( DiffuseColor.rgb * Intensity, DiffuseColor.a );
	
	finalfinal = gl_Color * finalfinal;
	
	finalfinal.rgb = vec3(.9) * finalfinal.rgb + vec3( .1 ) * skyColor.rgb;
	
	gl_FragColor = finalfinal;
}

