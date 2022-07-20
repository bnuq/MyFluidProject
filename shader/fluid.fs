#version 460 core

in float ViewRatio;


out vec4 fragColor;



void main()
{
    float alphaVal;
    if(ViewRatio < 0) alphaVal = 0;
    else              alphaVal = ViewRatio * 0.2;
        
    fragColor = vec4(1, 1, 0, alphaVal);
}