uniform sampler2D texUnit;
uniform float offset;
uniform vec2 halfpixel;

varying vec2 uv;

void main(void)
{
    vec4 sum = texture2D(texUnit, uv);
    sum += texture2D(texUnit, uv + vec2(halfpixel.x, 0.0) * offset);
    sum += texture2D(texUnit, uv + vec2(-halfpixel.x, 0.0) * offset);
    sum += texture2D(texUnit, uv + vec2(0.0, halfpixel.y) * offset);
    sum += texture2D(texUnit, uv + vec2(0.0, -halfpixel.y) * offset);
    gl_FragColor = sum / 5.0;
}
