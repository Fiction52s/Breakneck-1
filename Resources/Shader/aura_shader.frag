uniform sampler2D u_texture;

void main()
{
    vec4 pixel = texture2D(u_texture, gl_TexCoord[0].xy);
    gl_FragColor = gl_Color * pixel;
}