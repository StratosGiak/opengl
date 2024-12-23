#version 400

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoords;

uniform mat4 model;
uniform mat3 normalModel;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal;
out vec3 fragPos;
out vec2 textureCoords;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    normal = normalModel * aNormal;
    fragPos = vec3(model * vec4(aPos, 1.0));
    textureCoords = aTextureCoords;
}