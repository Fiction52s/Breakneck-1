uniform sampler2D u_texture;
uniform vec4 toColor;

void main()
{
    vec4 pixel = texture2D(u_texture, gl_TexCoord[0].xy);
	
    gl_FragColor = vec4( toColor.rgb, pixel.a );
}