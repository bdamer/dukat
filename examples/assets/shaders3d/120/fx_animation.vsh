#version 120
///
// FBO example vertex shader.
///
uniform float u_time;

void main()
{
    gl_Position = gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}