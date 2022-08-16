#include "Particle.h"


// Fluid �� �����ϴ�, ���� => (0, 0, 0) �� ��������, xyz �� ���̸� ����
glm::vec3 Particle::FluidRange = glm::vec3(60, 200, 60);

// �� ���� �����ϴ� ��ƼŬ�� ����
//glm::uvec3 Particle::ParticleCount = glm::uvec3(32, 16, 32);          // 2^14 = 16384 ��
//glm::uvec3 Particle::ParticleCount = glm::uvec3(50, 10, 50);          // 25000 ��
glm::uvec3 Particle::ParticleCount = glm::uvec3(32, 32, 32);            // 2^15 = 32768 ��
//glm::uvec3 Particle::ParticleCount = glm::uvec3(4, 4, 4);

// ��ü ��ƼŬ�� ����
unsigned int Particle::TotalParticleCount = (Particle::ParticleCount.x) * (Particle::ParticleCount.y) * (Particle::ParticleCount.z);



// ��ƼŬ���� ��� �׷����� �� �ʿ��� �׷� ����
unsigned int Particle::GroupNum = (Particle::TotalParticleCount % 32 == 0) ? (Particle::TotalParticleCount / 32) : (Particle::TotalParticleCount / 32) + 1;



// ���Ƿ� 1 ���� �����ٰ� ����
float Particle::ParticleMass = 1.0f;