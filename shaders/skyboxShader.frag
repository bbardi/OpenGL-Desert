#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform bool fog;
uniform samplerCube skybox;

void main()
{
    if (fog)
        color = vec4(0.75f, 0.7f, 0.5f, 1.0f);
    else
        color = texture(skybox, textureCoordinates);
}
