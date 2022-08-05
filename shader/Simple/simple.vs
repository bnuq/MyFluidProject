#version 460 core

layout (location = 0) in vec3 aPos;


struct CoreParticle
{
    float xpos;
    float ypos;
    float zpos;

    float xvel;
    float yvel;
    float zvel;

    float toCamera;
};
layout(std430, binding = 1) buffer CoreParticleBuffer
{
    CoreParticle cData[];
};


uniform mat4 transform;

void main()
{
    // ���� ������ ���� Core Particle Instance ��ȣ�� ����
    // �ڽ��� ���� Core Particle �� ������ ��´�
    CoreParticle curParticle = cData[gl_InstanceID];


    // �� ��ǥ�� -> ���� ��ǥ��, ��ȯ
    mat4 modelTransform = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            curParticle.xpos, curParticle.ypos, curParticle.zpos, 1.0
        );
    

    // Ŭ�� ���������� ��ġ
    gl_Position = transform * modelTransform * vec4(aPos, 1.0);
}