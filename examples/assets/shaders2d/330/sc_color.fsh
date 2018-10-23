#version 330
precision mediump float;
///
// Flat shaded fragment shader.
///
in vec4 v_color;

out vec4 o_color;

void main()
{
    o_color = v_color;
}