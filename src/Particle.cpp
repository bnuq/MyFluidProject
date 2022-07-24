#include "Particle.h"

Particle::Particle(glm::vec4 pos, glm::vec3 camPos)
: position( pos )
{
    toCamera = glm::distance(glm::vec3(pos.x, pos.y, pos.z), camPos);
}


// Fluid �� �����ϴ�, ���� => (0, 0, 0) �� ��������, xyz �� ���̸� ����
glm::vec3 Particle::FluidRange = glm::vec3(5, 5, 5);



// �� ���� �����ϴ� ��ƼŬ�� ����
glm::uvec3 Particle::ParticleCount = glm::uvec3(4, 2, 4);

// ��ü ��ƼŬ�� ����
unsigned int Particle::TotalParticleCount = (Particle::ParticleCount.x) * (Particle::ParticleCount.y) * (Particle::ParticleCount.z);


// 0.1f ~ 0.5f ���� ���� ������ ����
float Particle::ParticleMass = 0.25f;