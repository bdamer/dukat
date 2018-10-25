#version 300 es
precision mediump float;
/**
 * Fragment shader for debug rendering.
 */
// no inputs
 
uniform vec4 u_color;

out vec4 o_color;

void main()
{
    o_color = u_color;
}