attribute highp vec4 vertexCoords;
attribute mediump vec2 textureCoords;

uniform highp mat4 projectionMatrix;

varying mediump vec2 uv;
varying mediump vec2 vertex;

void main()
{
    gl_Position = projectionMatrix * vertexCoords;
    uv = textureCoords;
    vertex = textureCoords;
}
