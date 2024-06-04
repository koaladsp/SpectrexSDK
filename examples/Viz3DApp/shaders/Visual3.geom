#version 410 core

layout(points) in;
layout(triangle_strip, max_vertices=12) out;

in VertexData
{
    int InstanceID;
    float ShapeWidth;
    float ShapeLength;
    float SpectrogramValue;
}
In[1];

out GeometryData
{
    flat int InstanceID;
}
Out;

uniform mat4 uViewProjection;

uniform float uBaseHeight;
uniform float uYDisplacement;
uniform float uGlobalScale;

uniform float uLineSpectrumHeight;

mat4
translate(vec3 d)
{
    // clang-format off
    return mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    d, 1.0
    );
    // clang-format on
}

mat4
scale(vec3 c)
{
    // clang-format off
    return mat4(
    c.x, 0.0, 0.0, 0.0,
    0.0, c.y, 0.0, 0.0,
    0.0, 0.0, c.z, 0.0,
    0.0, 0.0, 0.0, 1.0
    );
    // clang-format on
}

void createFace(vec3 pos, vec3 up, vec3 right, mat4 m) {
    gl_Position = m * vec4(pos, 1);
    EmitVertex();

    gl_Position = m * vec4(pos + right, 1);
    EmitVertex();

    gl_Position = m * vec4(pos + up, 1);
    EmitVertex();

    gl_Position = m * vec4(pos + right + up, 1);
    EmitVertex();

    EndPrimitive();
}

void main() {
    Out.InstanceID = In[0].InstanceID;

    // invidual shape's size
    vec3 size = vec3(In[0].ShapeWidth, uBaseHeight + In[0].SpectrogramValue * uLineSpectrumHeight, In[0].ShapeLength);

    // sets the shape's bottom at y=0
    vec3 sizeOffset = vec3(0, size.y / 2.f, 0);

    mat4 m =
    uViewProjection *
    // entire visualization transformation
    translate(vec3(0, uYDisplacement, 0)) *
    scale(vec3(uGlobalScale)) *
    // invidual shape transformation
    translate(gl_in[0].gl_Position.xyz + sizeOffset) *
    scale(size);

    // generate geometry
    const vec3 faceData[9] = vec3[](
    vec3( -0.5f, -0.5f,  0.5f ), vec3( 0, 1, 0 ), vec3(  1, 0,  0 ),
    vec3( -0.5f, -0.5f, -0.5f ), vec3( 0, 1, 0 ), vec3(  0, 0,  1 ),
    vec3(  0.5f,  0.5f, -0.5f ), vec3( 0, 0, 1 ), vec3( -1, 0,  0 )
    );

    for (int i = 0; i < 3; i++) {
        vec3 facePos = faceData[i * 3 + 0];
        vec3 faceUp = faceData[i * 3 + 1];
        vec3 faceRight = faceData[i * 3 + 2];

        // creates face with outwards non uniform border scale
        createFace(facePos, faceUp, faceRight, m);
    }
}
