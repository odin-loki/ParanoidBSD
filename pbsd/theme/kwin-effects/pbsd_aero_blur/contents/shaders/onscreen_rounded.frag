uniform sampler2D texUnit;
uniform mat4 colorMatrix;
uniform float offset;
uniform vec2 halfpixel;
uniform vec4 box;
uniform vec4 cornerRadius;
uniform vec4 glassTint;
uniform float opacity;

varying vec2 uv;
varying vec2 vertex;

float roundedBox(vec2 p, vec2 b, vec4 r)
{
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x = (p.y > 0.0) ? r.x : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

void main(void)
{
    vec4 sum = texture2D(texUnit, uv + vec2(-halfpixel.x * 2.0, 0.0) * offset);
    sum += texture2D(texUnit, uv + vec2(-halfpixel.x, halfpixel.y) * offset) * 2.0;
    sum += texture2D(texUnit, uv + vec2(0.0, halfpixel.y * 2.0) * offset);
    sum += texture2D(texUnit, uv + vec2(halfpixel.x, halfpixel.y) * offset) * 2.0;
    sum += texture2D(texUnit, uv + vec2(halfpixel.x * 2.0, 0.0) * offset);
    sum += texture2D(texUnit, uv + vec2(halfpixel.x, -halfpixel.y) * offset) * 2.0;
    sum += texture2D(texUnit, uv + vec2(0.0, -halfpixel.y * 2.0) * offset);
    sum += texture2D(texUnit, uv + vec2(-halfpixel.x, -halfpixel.y) * offset) * 2.0;

    vec4 blurred = (sum / 12.0) * colorMatrix * opacity;
    blurred = mix(blurred, vec4(glassTint.rgb, 1.0), glassTint.a * 0.35);

    vec2 center = box.xy + box.zw * 0.5;
    float dist = roundedBox(vertex - center, box.zw * 0.5, cornerRadius);
    float alpha = 1.0 - smoothstep(-1.0, 1.0, dist);
    gl_FragColor = vec4(blurred.rgb, blurred.a * alpha);
}
