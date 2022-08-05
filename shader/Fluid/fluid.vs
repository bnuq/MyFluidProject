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



uniform mat4 transform;         // 모델 좌표계 => 클립 공간 좌표
uniform vec3 CameraPos;         // 카메라의 위치


out vec4 ClipPos;               // 클립 공간에서 좌표
out float ViewRatio;            // 카메라에서 보이는 정도



void main()
{
    // 현재 정점이 속한 particle instance 를 찾는다
    Particle curParticle = pData[gl_InstanceID];


    // 모델 좌표계 -> 월드 좌표계, 변환
    mat4 modelTransform = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            curParticle.position.x, curParticle.position.y, curParticle.position.z, 1.0
        );


    ClipPos = transform * modelTransform * vec4(aPos, 1.0);
    gl_Position = ClipPos;


    // 월드 좌표계에서의 값
    vec3 WorldPos = ( modelTransform * vec4(aPos, 1.0) ).xyz;
    vec3 WorldNormal = aNormal;  // 모델변환에 이동만 있으므로, 노멀 벡터는 동일하다

    
    // 정점에서 카메라를 바라보는 뷰 벡터
    vec3 ViewVec = normalize(CameraPos - WorldPos);
    

    // 카메라에서 해당 정점이 보이는 정도
    ViewRatio = dot(WorldNormal, ViewVec);
}