#version 330 core
// Minimal GL version support expected from the GPU

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 viewMat, projMat, trans;
uniform vec4 position; // Uniform
uniform vec3 color;

out vec2 TexCoord;
out vec3 fNormal;
out vec3 fPosition; // useful?
out vec3 fPositionSphereCenter;

void main() {

    fNormal = vNormal;
    TexCoord = aTexCoord;
    fPosition = vPosition;
    fPositionSphereCenter = position.xyz;
    gl_Position = projMat * viewMat * trans * position; // mandatory to rasterize properly

}
