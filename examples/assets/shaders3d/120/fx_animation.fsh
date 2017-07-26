#version 120
///
// FBO example fragment shader.
///
uniform float u_time;

void main()
{
    gl_FragColor = vec4(sin(u_time * gl_TexCoord[0].s), cos(u_time * gl_TexCoord[0].t), 0, 1);
}