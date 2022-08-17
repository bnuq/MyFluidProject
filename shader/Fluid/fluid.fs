#version 460 core


uniform sampler2D depthMap;
uniform float offset;
uniform float alphaOffset;
uniform vec3 CameraDir;             // ī�޶� �ٶ󺸴� ����


in vec4 ClipPos;
in vec3 ViewVec;
in vec3 WorldNormal;

out vec4 fragColor;


void main()
{
    // Ŭ�� ���� ��ǥ�� �̿��ؼ�, depth map ���� ���� �о�� �ؽ�ó ��ǥ�� ���Ѵ�
    vec3 depthMapCoord = ClipPos.xyz / ClipPos.w;       // NDC ��ǥ => [-1, +1] ����
    depthMapCoord = depthMapCoord * 0.5 + 0.5;          // [0, 1] ������ ����


    // depth map ���� ���̸� �����ͼ�, ���� fragment ���̿� ��
    float closestDepth = texture(depthMap, depthMapCoord.xy).r;
    float currentDepth = depthMapCoord.z;


    // �޸��̸� �׸��� �ʴ´�
    if(!gl_FrontFacing)
        discard;

    
    // ���� ���̰� offset �̻��̸� �׸��� �ʴ´�
    if(abs((closestDepth - currentDepth)) > offset)
        discard;
    // ���̰� ������ frag color �� �����Ѵ�
    else
    {   
        // �ش� fragment ���� ī�޶� ���̴� ����
        // �����׸�Ʈ�� ī�޶� �ٶ󺸴� ���Ϳ�
        // ī�޶� �ٶ󺸴� ���͸� ���Ѵ� => ���̳ʽ� ���͸� ����
        // ���� �������� �ٶ󺼼��� ���� ����� 1 �� ���´�
//        float ViewRatio = max( dot(normalize(CameraDir), normalize(ViewVec)), 0.0 );

        float ViewRatio = max( dot(normalize(-CameraDir), normalize(WorldNormal)), 0.0 );


        // ������ �Ķ���
        // ������ ������ �� �ְ� �Ѵ�
        // ���鿡 ���ϼ��� ������ ������ �Ѵ�
        fragColor = vec4(0, 0, 1, (1 - ViewRatio) * alphaOffset);
    }
}