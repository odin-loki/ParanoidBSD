attribute highp vec4 vertexCoords;
attribute mediump vec2 textureCoords;

uniform highp mat4 projectionMatrix;

varying mediump vec2 uv;

void main()
{
    gl_Position = projectionMatrix * vertexCoords;
    uv = textureCoords;
}
