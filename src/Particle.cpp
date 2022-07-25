#include "Particle.h"



// Fluid �� �����ϴ�, ���� => (0, 0, 0) �� ��������, xyz �� ���̸� ����
glm::vec3 Particle::FluidRange = glm::vec3(10, 5, 10);



// �� ���� �����ϴ� ��ƼŬ�� ����
glm::uvec3 Particle::ParticleCount = glm::uvec3(8, 4, 8);

// ��ü ��ƼŬ�� ����
unsigned int Particle::TotalParticleCount = (Particle::ParticleCount.x) * (Particle::ParticleCount.y) * (Particle::ParticleCount.z);


unsigned int Particle::GroupNum = Particle::TotalParticleCount / 32;


// 0.1f ~ 0.5f ���� ���� ������ ����
float Particle::ParticleMass = 1.0f;