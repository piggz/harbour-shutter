out vec4 FragColor;
in vec2 textureOut;
uniform sampler2D tex_y;
uniform vec2 tex_step;

void main(void)
{

    vec2 pos = textureOut;
    float f_x = fract(pos.x / tex_step.x);

    vec4 left = texture2D(tex_y, vec2(pos.x - f_x * tex_step.x, pos.y));
    vec4 right = texture2D(tex_y, vec2(pos.x + (1.0 - f_x) * tex_step.x , pos.y));

    float y_left = mix(left.r, left.b, f_x * 2.0);
    float y_right = mix(left.b, right.r, f_x * 2.0 - 1.0);
    vec2 uv = mix(left.ga, right.ga, f_x);

    float y = mix(y_left, y_right, step(0.5, f_x));

    //vec3 rgb = yuv2rgb_matrix * (vec3(y, uv) - yuv2rgb_offset);

    vec3 rgb = texture2D(tex_y, textureOut).rgb;

    //vec3 rgb = vec3(0.0, 1.0, 0.5);

    FragColor = vec4(rgb, 1.0);
}
