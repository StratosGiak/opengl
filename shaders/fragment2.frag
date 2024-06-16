#version 400

in vec2 textureCoords;
uniform sampler2D texture_diffuse0;

out vec4 fragColor;

void main() {
    fragColor = texture(texture_diffuse0, textureCoords);
}