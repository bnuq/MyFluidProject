#include "Particle.h"


// Fluid 가 존재하는, 영역 => (0, 0, 0) 를 기준으로, xyz 로 길이를 지정
glm::vec3 Particle::FluidRange = glm::vec3(16.5, 10, 8.5);

// 한 축을 구성하는 파티클의 개수
glm::uvec3 Particle::ParticleCount = glm::uvec3(16, 8, 8);

// 전체 파티클의 개수
unsigned int Particle::TotalParticleCount = (Particle::ParticleCount.x) * (Particle::ParticleCount.y) * (Particle::ParticleCount.z);

// 파티클들을 모두 그려내는 데 필요한 그룹 개수
unsigned int Particle::GroupNum = (Particle::TotalParticleCount % 32 == 0) ? (Particle::TotalParticleCount / 32) : (Particle::TotalParticleCount / 32) + 1;


// 임의로 1 값을 가진다고 가정
float Particle::ParticleMass = 0.1f;