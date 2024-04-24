#version 450

layout(binding = 0) uniform Matrix {mat4 buf[];} inMat;

#if defined(vertexPractice)
layout(location = 0) out vec3 fragColor;
layout(location = 0) in vec2 inVec;
layout(location = 1) in uint inRef;
void vertexPractice() {
    // TODO change to inMat.buf[uni.pro] * inMat.buf[uni.all] * inMat.buf[tri.tri[gl_VertexID/3].pol] * vec4(vtx[tri.tri[gl_VertexID/3].vtx[gl_VertexID%3]].vtx,1.0)
    gl_Position = inMat.buf[0] * vec4(inVec, 0.5, 1.0);
    // TODO change to vtx[tri.tri[gl_VertexIndex/3].vtx[gl_VertexIndex%3]].clr
    switch (gl_VertexIndex%6) {
    case (0): fragColor = fragColor = vec3(1.0,0.0,0.0); break;
    case (1): fragColor = fragColor = vec3(0.0,0.0,1.0); break;
    case (2): fragColor = fragColor = vec3(0.0,0.0,1.0); break;
    case (3): fragColor = fragColor = vec3(1.0,0.0,0.0); break;
    case (4): fragColor = fragColor = vec3(0.0,0.0,1.0); break;
    case (5): fragColor = fragColor = vec3(0.0,0.0,1.0); break;
    }
}
#endif

#if defined(fragmentCombine)
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;
void fragmentCombine() {
    outColor = vec4(fragColor, 1.0);
}
#endif
