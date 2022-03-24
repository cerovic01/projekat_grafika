#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
    vec4 result = texture(texture_diffuse1, TexCoords);
    result += vec4(1.0, 0.5, 0.0, 1.0);

    vec3 r = vec3(result);
    float brightness = dot(r, vec3(0.2126, 0.7152, 0.0722));
        if(brightness > 1.0)
            BrightColor = vec4(r, 1.0);
        else
            BrightColor = vec4(0.0, 0.0, 0.0, 1.0);


    FragColor = result;
}