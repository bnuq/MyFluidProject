#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;


struct CoreParticle
{
    float xpos;
    float ypos;
    float zpos;

    float xvel;
    float yvel;
    float zvel;

    float toCamera;

    uint visible;
};
layout(std430, binding = 1) buffer CoreParticleBuffer
{
    CoreParticle cData[];
};



uniform mat4 transform;         // �� ��ǥ�� => Ŭ�� ���� ��ǥ
uniform vec3 CameraPos;         // ī�޶��� ��ġ


out vec4 ClipPos;               // Ŭ�� �������� ��ǥ
out float ViewRatio;            // ī�޶󿡼� ���̴� ����



void main()
{
    // ���� ������ ���� core particle instance �� ã�´�
    CoreParticle curParticle = cData[gl_InstanceID];


    // �� ��ǥ�� -> ���� ��ǥ��, ��ȯ
    mat4 modelTransform = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            curParticle.xpos, curParticle.ypos, curParticle.zpos, 1.0
        );


    ClipPos = transform * modelTransform * vec4(aPos, 1.0);
    gl_Position = ClipPos;


    // �������� ī�޶� �ٶ󺸴� �� ����
    vec3 ViewVec = CameraPos - vec3(curParticle.xpos, curParticle.ypos, curParticle.zpos);
    ViewVec = normalize(ViewVec);
    ViewRatio = dot(aNormal, ViewVec);
}