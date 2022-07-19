#include "Particle.h"

Particle::Particle(glm::vec4 pos, glm::vec3 camPos)
: Position( pos )
{
    ToCamera = glm::distance(glm::vec3(Position.x, Position.y, Position.z), camPos);
}


// Fluid 가 존재하는, 영역 => (0, 0, 0) 를 기준으로, xyz 로 길이를 지정
glm::vec3 Particle::FluidRange = glm::vec3(10, 8, 6);



// 한 축을 구성하는 파티클의 개수
glm::uvec3 Particle::ParticleCount = glm::uvec3(2, 2, 2);

// 전체 파티클의 개수
int Particle::TotalParticleCount = (Particle::ParticleCount.x) * (Particle::ParticleCount.y) * (Particle::ParticleCount.z);