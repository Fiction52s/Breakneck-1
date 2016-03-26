//boundary fragment program

#version 120

uniform vec2 offset;
uniform float scale; //scale param
uniform sampler2D x; //state field
uniform vec2 texSize;

void main()
{	
	gl_FragColor = scale * texture2D( x, (gl_FragCoord.xy + offset) / texSize  );
}