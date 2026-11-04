uniform sampler2D texUnit;
uniform float offset;
uniform vec2 halfpixel;

varying vec2 uv;

void main(void)
{
    gl_FragColor = texture2D(texUnit, uv + halfpixel * offset);
}
