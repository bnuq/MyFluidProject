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
    // 현재 정점이 속한 Core Particle Instance 번호로 접근
    // 자신이 속한 Core Particle 의 정보를 얻는다
    CoreParticle curParticle = cData[gl_InstanceID];


    // 모델 좌표계 -> 월드 좌표계, 변환
    mat4 modelTransform = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            curParticle.xpos, curParticle.ypos, curParticle.zpos, 1.0
        );
    

    // 클립 공간에서의 위치
    gl_Position = transform * modelTransform * vec4(aPos, 1.0);
}