uniform sampler2D texUnit;
uniform float noiseStrength;
uniform vec2 halfpixel;

varying vec2 uv;

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main(void)
{
    vec4 color = texture2D(texUnit, uv);
    float n = (rand(uv * halfpixel) - 0.5) * noiseStrength;
    gl_FragColor = vec4(color.rgb + n, color.a);
}
