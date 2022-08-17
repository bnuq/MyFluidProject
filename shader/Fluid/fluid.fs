#version 460 core


uniform sampler2D depthMap;
uniform float offset;
uniform float alphaOffset;
uniform vec3 CameraDir;             // 카메라가 바라보는 방향


in vec4 ClipPos;
in vec3 ViewVec;
in vec3 WorldNormal;

out vec4 fragColor;


void main()
{
    // 클립 공간 좌표를 이용해서, depth map 에서 값을 읽어올 텍스처 좌표를 구한다
    vec3 depthMapCoord = ClipPos.xyz / ClipPos.w;       // NDC 좌표 => [-1, +1] 범위
    depthMapCoord = depthMapCoord * 0.5 + 0.5;          // [0, 1] 범위로 변경


    // depth map 에서 깊이를 가져와서, 현재 fragment 깊이와 비교
    float closestDepth = texture(depthMap, depthMapCoord.xy).r;
    float currentDepth = depthMapCoord.z;


    // 뒷면이면 그리지 않는다
    if(!gl_FrontFacing)
        discard;

    
    // 깊이 차이가 offset 이상이면 그리지 않는다
    if(abs((closestDepth - currentDepth)) > offset)
        discard;
    // 깊이가 같으면 frag color 를 리턴한다
    else
    {   
        // 해당 fragment 면이 카메라에 보이는 정도
        // 프레그먼트가 카메라를 바라보는 벡터와
        // 카메라가 바라보는 벡터를 비교한다 => 마이너스 벡터를 취해
        // 서로 정면으로 바라볼수록 내적 결과가 1 이 나온다
//        float ViewRatio = max( dot(normalize(CameraDir), normalize(ViewVec)), 0.0 );

        float ViewRatio = max( dot(normalize(-CameraDir), normalize(WorldNormal)), 0.0 );


        // 무조건 파란색
        // 투명도를 조절할 수 있게 한다
        // 정면에 보일수록 투명해 보여야 한다
        fragColor = vec4(0, 0, 1, (1 - ViewRatio) * alphaOffset);
    }
}