#version 460 core


uniform sampler2D depthMap;
uniform float offset;


in vec4 ClipPos;
in float ViewRatio;

out vec4 fragColor;



void main()
{
    // 클립 공간 좌표를 이용해서, depth map 에서 값을 읽어올 텍스처 좌표를 구한다
    vec3 depthMapCoord = ClipPos.xyz / ClipPos.w;       // NDC 좌표
    depthMapCoord = depthMapCoord * 0.5 + 0.5;          // [0, 1] 범위로 변경



    // depth map 에서 깊이를 가져와서, 현재 fragment 깊이와 비교
    float closestDepth = texture(depthMap, depthMapCoord.xy).r;
    float currentDepth = depthMapCoord.z;


    
    // 깊이 차이가 offset 이상이면 그리지 않는다
    if(abs((closestDepth - currentDepth)) > offset)
        discard;
    // 깊이가 같으면 frag color 를 리턴한다
    else
    {
        float alphaVal;
        if(ViewRatio < 0) alphaVal = 0;
        else              alphaVal = ViewRatio * 0.2;
            
        fragColor = vec4(0, 0, 1, alphaVal);
    }
}