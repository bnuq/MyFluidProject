#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "../FrameWork/common.h"


// 정렬의 대상이 되는 타입
// CPU -> GPU 사이로 주고 받는 데이터 형식, 꼭 필요한 것만 넣었다
class CoreParticle
{
    public:
    CoreParticle(glm::vec3 pos, glm::vec3 camPos)
    {
        xpos = pos.x; ypos = pos.y; zpos = pos.z; 
        xvel = 0.0f;  yvel = 0.0f;  zvel = 0.0f;

        toCamera = glm::distance(camPos, pos);
    }

    float xpos, ypos, zpos;
    float xvel, yvel, zvel;
    float toCamera;
};




class Particle
{
public:
    // std430 Memory Layout => vec4 length
    glm::vec4 position     = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::vec4 velocity     = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    glm::uvec2 range       = glm::uvec2(0, 0);
    float density          = 0.0f;
    float pressure         = 0.0f;

    glm::vec3 force        = glm::vec3(0.0f, 0.0f, 0.0f);         // net force
    unsigned int neighbor  = 0;
    
    glm::vec3 surfNormal   = glm::vec3(0.0f, 0.0f, 0.0f);           // 유체를 향하는 surface normal 벡터
    unsigned int isSurface = 0;                                     // 표면 여부를 확인, surf normal 크기가 일정이상 넘으면 1 을 가진다



    /* Static Members */

    // Fluid 가 담기는 영역
    static glm::vec3 FluidRange;
    
    // Particle 들이 몇 개 존재하는 지, 영역
    static glm::uvec3 ParticleCount;
    
    // Particle 들의 총 개수
    static unsigned int TotalParticleCount;


    // 1 group = 32 thread => 필요한 그룹 수
    static unsigned int GroupNum;


    // Particle 의 질량
    static float ParticleMass;
};




#endif // __PARTICLE_H__