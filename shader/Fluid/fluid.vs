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



uniform mat4 transform;         // 모델 좌표계 => 클립 공간 좌표
uniform vec3 CameraPos;         // 카메라의 위치


out vec4 ClipPos;               // 클립 공간에서 좌표
out vec3 ViewVec;
out vec3 WorldNormal;

void main()
{
    // 현재 정점이 속한 Core Particle instance 를 찾는다
    CoreParticle curParticle = cData[gl_InstanceID];


    // 모델 좌표계 -> 월드 좌표계, 변환
    mat4 modelTransform = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            curParticle.xpos, curParticle.ypos, curParticle.zpos, 1.0
        );


    ClipPos = transform * modelTransform * vec4(aPos, 1.0);
    gl_Position = ClipPos;
   
    vec3 WorldPos = ( modelTransform * vec4(aPos, 1.0) ).xyz;       // 월드 좌표계에서의 값
    ViewVec = normalize(CameraPos - WorldPos);      // 정점에서 카메라를 바라보는 뷰 벡터

    WorldNormal = aNormal;
}