#version 460 core


uniform sampler2D depthMap;
uniform float offset;


in vec4 ClipPos;
in float ViewRatio;

out vec4 fragColor;



void main()
{
    // Ŭ�� ���� ��ǥ�� �̿��ؼ�, depth map ���� ���� �о�� �ؽ�ó ��ǥ�� ���Ѵ�
    vec3 depthMapCoord = ClipPos.xyz / ClipPos.w;       // NDC ��ǥ
    depthMapCoord = depthMapCoord * 0.5 + 0.5;          // [0, 1] ������ ����



    // depth map ���� ���̸� �����ͼ�, ���� fragment ���̿� ��
    float closestDepth = texture(depthMap, depthMapCoord.xy).r;
    float currentDepth = depthMapCoord.z;


    
    // ���� ���̰� offset �̻��̸� �׸��� �ʴ´�
    if(abs((closestDepth - currentDepth)) > offset)
        discard;
    // ���̰� ������ frag color �� �����Ѵ�
    else
    {
        float alphaVal;
        if(ViewRatio < 0) alphaVal = 0;
        else              alphaVal = ViewRatio * 0.2;
            
        fragColor = vec4(0, 0, 1, alphaVal);
    }
}