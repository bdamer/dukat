#version 330
precision mediump float;
///
// Wave effect fragment shader.
///
// Texture coordinates
in vec2 v_tex_coord;
// 4 per-wave coordinates to lookup cos values
in vec4 v_cos_coord;

uniform vec4 u_rescale; // used to scale back to 0..1
uniform vec4 u_scale_bias; // ?
uniform mat4 u_rtex_coord; // ripple texture coords
uniform mat4 u_coef; // wave coefficients
uniform float u_pass; // index of current pass

// Consine lookup table
uniform sampler1D u_tex0;
// Noise map
uniform sampler2D u_tex1;
// Normal map
uniform sampler2D u_tex2;

out vec4 o_color;

void main()
{
    // sample previous pass
    vec4 dest_color = texture(u_tex2, v_tex_coord);

    // wave passes [0..3]
    if (u_pass < 4.0)
    {
        // Sample cosine for each wave
        vec4 t0 = texture(u_tex0, v_cos_coord.x);
        vec4 t1 = texture(u_tex0, v_cos_coord.y);
        vec4 t2 = texture(u_tex0, v_cos_coord.z);
        vec4 t3 = texture(u_tex0, v_cos_coord.w);
        vec4 tmp =  2.0 * (t0 - 0.5) * u_coef[0] +
                    2.0 * (t1 - 0.5) * u_coef[1] +
                    2.0 * (t2 - 0.5) * u_coef[2] +
                    2.0 * (t3 - 0.5) * u_coef[3];
        
        // Bias tmp back into range [0..1]
        vec4 c1 = tmp * u_rescale + u_rescale;

        // first pass just overrides existing content, later passes use additive blending
        o_color = c1 + dest_color * min(u_pass, 1.0);
    }
    // noise pass [4]
    else
    {
        // sample noise texture
        vec4 t0 = texture(u_tex1, v_cos_coord.xy);
        vec4 t1 = texture(u_tex1, v_cos_coord.zw);
        vec4 c1 = vec4(t0.rgb + t1.rgb - 1, t0.a + t1.a);
        // rescale with biased scale
        c1.rgb = c1.rgb * u_rescale.xyz + u_rescale.xyz;
        o_color = c1 + dest_color;
    }
}