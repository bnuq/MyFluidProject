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

    float toOrigin;
};
layout(std430, binding = 1) buffer CoreParticleBuffer
{
    CoreParticle cData[];
};



uniform mat4 transform;         // �� ��ǥ�� => Ŭ�� ���� ��ǥ
uniform vec3 CameraPos;         // ī�޶��� ��ġ


out vec4 ClipPos;               // Ŭ�� �������� ��ǥ
out vec3 ViewVec;
out vec3 WorldNormal;

void main()
{
    // ���� ������ ���� Core Particle instance �� ã�´�
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
   
    vec3 WorldPos = ( modelTransform * vec4(aPos, 1.0) ).xyz;       // ���� ��ǥ�迡���� ��
    ViewVec = normalize(CameraPos - WorldPos);      // �������� ī�޶� �ٶ󺸴� �� ����

    WorldNormal = aNormal;
}