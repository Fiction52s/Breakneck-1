#version 120

uniform sampler2D u_texture;
uniform sampler2D u_normals;   //normal map
uniform sampler2D u_pattern;

uniform vec2 topLeft;
uniform float zoom;

//layout(origin_upper_left) in vec4 gl_FragCoord;

//this makes things weird sometimes. need y coords reversed in a way.

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen

uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 

void main()
{
	float size = 512.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	
	vec2 pos = mod( topLeft + pixelPos, size) / vec2( size );
	
	vec4 finalfinal = vec4( 0, 0, 0, 0 );
	
	vec4 DiffuseColor = texture2D(u_texture, pos);
		
	vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
	vec3 Intensity = Ambient;//.000001;
	finalfinal = vec4( DiffuseColor.rgb * Intensity, DiffuseColor.a );
	
	gl_FragColor =  gl_Color * finalfinal;//vec4(finalfinal, DiffuseColor.a);	
}

