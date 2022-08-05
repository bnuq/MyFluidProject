#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;


struct Particle
{
    vec4 position;

    vec4 velocity;

    uvec2 range;
    float density;
    float pressure;

    vec3 force;
    uint neighbor;

    vec3 surfNormal;
    uint isSurface;
};
layout(std430, binding = 2) buffer ParticleBuffer
{
    Particle pData[];
};



uniform mat4 transform;         // �� ��ǥ�� => Ŭ�� ���� ��ǥ
uniform vec3 CameraPos;         // ī�޶��� ��ġ


out vec4 ClipPos;               // Ŭ�� �������� ��ǥ
out float ViewRatio;            // ī�޶󿡼� ���̴� ����



void main()
{
    // ���� ������ ���� particle instance �� ã�´�
    Particle curParticle = pData[gl_InstanceID];


    // �� ��ǥ�� -> ���� ��ǥ��, ��ȯ
    mat4 modelTransform = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            curParticle.position.x, curParticle.position.y, curParticle.position.z, 1.0
        );


    ClipPos = transform * modelTransform * vec4(aPos, 1.0);
    gl_Position = ClipPos;


    // ���� ��ǥ�迡���� ��
    vec3 WorldPos = ( modelTransform * vec4(aPos, 1.0) ).xyz;
    vec3 WorldNormal = aNormal;  // �𵨺�ȯ�� �̵��� �����Ƿ�, ��� ���ʹ� �����ϴ�

    
    // �������� ī�޶� �ٶ󺸴� �� ����
    vec3 ViewVec = normalize(CameraPos - WorldPos);
    

    // ī�޶󿡼� �ش� ������ ���̴� ����
    ViewRatio = dot(WorldNormal, ViewVec);
}