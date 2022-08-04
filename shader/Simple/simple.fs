#version 460 core

// 색깔은 받자
uniform vec3 MainColor;


out vec4 fragColor;

void main()
{
    fragColor = vec4(MainColor, 1);
}