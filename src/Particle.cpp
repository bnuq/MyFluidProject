#include "Particle.h"

Particle::Particle(glm::vec3 pos)
: Position( pos )
{
    
}


// Fluid �� �����ϴ�, ���� => (0, 0, 0) �� ��������, xyz �� ���̸� ����
glm::vec3 Particle::FluidRange = glm::vec3(5, 4, 3);



// �� ���� �����ϴ� ��ƼŬ�� ����
glm::uvec3 Particle::ParticleCount = glm::uvec3(2, 2, 2);

// ��ü ��ƼŬ�� ����
int Particle::TotalParticleCount = (Particle::ParticleCount.x) * (Particle::ParticleCount.y) * (Particle::ParticleCount.z);