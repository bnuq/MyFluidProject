#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "common.h"


class Particle
{
public:
    Particle(glm::vec3 pos);

    glm::vec3 Position  = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 Velocity  = glm::vec3(0.0f, 0.0f, 0.0f);

    float density       = 0.0f;
    float pressure      = 0.0f;

    //particle 에 가해지는 힘 => 최소 3가지 요소로 구성되어 있다
    //압력 + 점성 + 표면장력
    glm::vec3 force     = glm::vec3(0.0f, 0.0f, 0.0f);
        
    //particle surface normal, 표면장력
    glm::vec3 surfNormal = glm::vec3(0.0f, 0.0f, 0.0f);

    static int ParticleCount;
};




#endif // __PARTICLE_H__