#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "common.h"


class Particle
{
public:
    Particle() {}
    Particle(glm::vec4 pos, glm::vec3 camPos);


    // std430 Memory Layout => vec4 length
    glm::vec4 position   = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::vec4 velocity   = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    glm::uvec2 range      = glm::uvec2(0, 0);
    float density        = 0.0f;
    float pressure       = 0.0f;

    glm::vec3 surfNormal = glm::vec3(0.0f, 0.0f, 0.0f);  // particle surface normal
    float isSurf         = 0.0f;                         // 0.0f == not surface

    glm::vec3 force     = glm::vec3(0.0f, 0.0f, 0.0f);   // net force
    float toCamera      = 0.0f;
    
    


    // Fluid 가 담기는 영역
    static glm::vec3 FluidRange;
    
    // Particle 들이 몇 개 존재하는 지, 영역
    static glm::uvec3 ParticleCount;
    
    // Particle 들의 총 개수
    static unsigned int TotalParticleCount;

    // Particle 의 질량
    static const float ParticleMass;
};




#endif // __PARTICLE_H__