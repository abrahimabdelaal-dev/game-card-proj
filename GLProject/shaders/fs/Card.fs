#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform vec3 objectColor;
uniform sampler2D texture_diffuse1;

void main()
{
    vec4 t = texture(texture_diffuse1, texCoord);
    // transparent pixels (e.g. the torn part of the poster) are not drawn at all
    if (t.a < 0.5)
        discard;
    FragColor = vec4(t.rgb * objectColor, 1.0);
}
