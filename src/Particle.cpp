#include "Particle.h"

Particle::Particle(glm::vec4 pos, glm::vec3 camPos)
: Position( pos )
{
    ToCamera = glm::distance(glm::vec3(Position.x, Position.y, Position.z), camPos);
}


// Fluid �� �����ϴ�, ���� => (0, 0, 0) �� ��������, xyz �� ���̸� ����
glm::vec3 Particle::FluidRange = glm::vec3(20, 15, 10);



// �� ���� �����ϴ� ��ƼŬ�� ����
glm::uvec3 Particle::ParticleCount = glm::uvec3(12, 12, 6);

// ��ü ��ƼŬ�� ����
int Particle::TotalParticleCount = (Particle::ParticleCount.x) * (Particle::ParticleCount.y) * (Particle::ParticleCount.z);