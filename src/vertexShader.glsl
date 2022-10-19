#version 330 core
// Minimal GL version support expected from the GPU

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

uniform mat4 viewMat, projMat;
uniform mat4 trans;
out vec3 new_position;
in vec4 position;

out vec3 fNormal;

void main() {
    gl_Position = projMat * viewMat * trans * position; // mandatory to rasterize properly

    fNormal = vNormal;
}
