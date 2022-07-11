#include "common.h"
#include <fstream>  // File Stream
#include <sstream>  // String Stream

// std::optional => value 를 리턴하면, 자동으로 optional 을 감싸서 리턴한다
std::optional<std::string> LoadTextFile(const std::string& filename)
{    
    // ifstream = file loading,  file -> stream
    std::ifstream fin(filename);
    if (!fin.is_open())
    {
        SPDLOG_ERROR("failed to open file: {}", filename);
        return std::nullopt;
    }

    std::stringstream text{};

    // ifstream::rdbuf() => stream buffer 를 리턴 => String Stream 에 넣을 수 있다
    text << fin.rdbuf();

    // stringstream::str() => 버퍼에 저장된 string 을 리턴
    return text.str();
}

glm::vec3 GetAttenuationCoeff(float distance)
{
    const auto linear_coeff = glm::vec4(
        8.4523112e-05, 4.4712582e+00, -1.8516388e+00, 3.3955811e+01
    );
    const auto quad_coeff = glm::vec4(
        -7.6103583e-04, 9.0120201e+00, -1.1618500e+01, 1.0000464e+02
    );

    float d = 1.0f / distance;
    auto dvec = glm::vec4(1.0f, d, d*d, d*d*d);

    float kc = 1.0f;
    float kl = glm::dot(linear_coeff, dvec);
    float kq = glm::dot(quad_coeff, dvec);

    // 상수로 이뤄진 벡터를 리턴
    return glm::vec3(kc, glm::max(kl, 0.0f), glm::max(kq*kq, 0.0f));
}