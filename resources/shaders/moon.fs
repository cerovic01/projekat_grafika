#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
    vec4 result = texture(texture_diffuse1, TexCoords);
    result += vec4(1.0, 0.5, 0.0, 1.0);
    FragColor = result;
}