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

    //particle �� �������� �� => �ּ� 3���� ��ҷ� �����Ǿ� �ִ�
    //�з� + ���� + ǥ�����
    glm::vec3 force     = glm::vec3(0.0f, 0.0f, 0.0f);
        
    //particle surface normal, ǥ�����
    glm::vec3 surfNormal = glm::vec3(0.0f, 0.0f, 0.0f);



    // Fluid �� ���� ����
    static glm::vec3 FluidRange;
    
    // Particle ���� �� �� �����ϴ� ��, ����
    static glm::uvec3 ParticleCount;
    
    // Particle ���� �� ���� => ���� ����
    static int TotalParticleCount;
};




#endif // __PARTICLE_H__