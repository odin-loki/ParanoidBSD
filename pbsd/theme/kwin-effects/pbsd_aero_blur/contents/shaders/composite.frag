uniform sampler2D texUnit;
uniform vec4 glassTint;
uniform float panelOpacity;
uniform float saturationBoost;
uniform float contrastBoost;

varying vec2 uv;

void main(void)
{
    vec4 color = texture2D(texUnit, uv);
    vec3 saturated = mix(vec3(dot(color.rgb, vec3(0.299, 0.587, 0.114))), color.rgb, saturationBoost);
    vec3 contrasted = (saturated - 0.5) * contrastBoost + 0.5;
    vec3 tinted = mix(contrasted, glassTint.rgb, glassTint.a * 0.25);
    gl_FragColor = vec4(tinted, color.a * panelOpacity);
}
