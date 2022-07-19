#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "common.h"


class Particle
{
public:
    Particle(glm::vec4 pos, glm::vec3 camPos);

    glm::vec4 Position  = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec4 Velocity  = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    //particle �� �������� �� => �ּ� 3���� ��ҷ� �����Ǿ� �ִ�
    //�з� + ���� + ǥ�����
    glm::vec4 force     = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        
    //particle surface normal, ǥ�����
    glm::vec4 surfNormal = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    
    /* 
        x : density       = 0.0f;
        y : pressure      = 0.0f;
        z : ToCamera      = 0.0f;
     */
    glm::vec4 property = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);



    // Fluid �� ���� ����
    static glm::vec3 FluidRange;
    
    // Particle ���� �� �� �����ϴ� ��, ����
    static glm::uvec3 ParticleCount;
    
    // Particle ���� �� ���� => ���� ����
    static int TotalParticleCount;
};




#endif // __PARTICLE_H__