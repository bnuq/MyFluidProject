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
    
    


    // Fluid �� ���� ����
    static glm::vec3 FluidRange;
    
    // Particle ���� �� �� �����ϴ� ��, ����
    static glm::uvec3 ParticleCount;
    
    // Particle ���� �� ����
    static unsigned int TotalParticleCount;

    // Particle �� ����
    static const float ParticleMass;
};




#endif // __PARTICLE_H__