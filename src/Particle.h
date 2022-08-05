#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "../FrameWork/common.h"


// ������ ����� �Ǵ� Ÿ��
// CPU -> GPU ���̷� �ְ� �޴� ������ ����, �� �ʿ��� �͸� �־���
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
    
    glm::vec3 surfNormal   = glm::vec3(0.0f, 0.0f, 0.0f);           // ��ü�� ���ϴ� surface normal ����
    unsigned int isSurface = 0;                                     // ǥ�� ���θ� Ȯ��, surf normal ũ�Ⱑ �����̻� ������ 1 �� ������



    /* Static Members */

    // Fluid �� ���� ����
    static glm::vec3 FluidRange;
    
    // Particle ���� �� �� �����ϴ� ��, ����
    static glm::uvec3 ParticleCount;
    
    // Particle ���� �� ����
    static unsigned int TotalParticleCount;


    // 1 group = 32 thread => �ʿ��� �׷� ��
    static unsigned int GroupNum;


    // Particle �� ����
    static float ParticleMass;
};




#endif // __PARTICLE_H__