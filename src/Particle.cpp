#include "Particle.h"

Particle::Particle(glm::vec4 pos, glm::vec3 camPos)
: Position( pos )
{
    ToCamera = glm::distance(glm::vec3(pos.x, pos.y, pos.z), camPos);
}


// Fluid �� �����ϴ�, ���� => (0, 0, 0) �� ��������, xyz �� ���̸� ����
glm::vec3 Particle::FluidRange = glm::vec3(1, 1, 1);



// �� ���� �����ϴ� ��ƼŬ�� ����
glm::uvec3 Particle::ParticleCount = glm::uvec3(4, 2, 4);

// ��ü ��ƼŬ�� ����
int Particle::TotalParticleCount = (Particle::ParticleCount.x) * (Particle::ParticleCount.y) * (Particle::ParticleCount.z);


float Particle::ParticleMass = 0.25f;