/*
    공통적으로 사용하는 헤더파일
    공통적으로 사용하는 기능들을
    모아서 정의한 파일
 */

/*  
    헤더 파일 내용을 한번만 include 시키는 기능
    #pragma once 는 windows 에서만 가능
 */
#ifndef __COMMON_H__    // if not define
#define __COMMON_H__    // define

#include <memory>
#include <string>
/* 
    std::optional<>
    어떤 값이 있거나 없는 경우를 포인터 없이 표현
    결과 값이 있을 수도 있고, 없을 수도 있는 경우를 다룬다
 */
#include <optional>

// 외부 라이브러리 추가
// 다른 모든 클래스에서도 사용할 수 있게 하기 위해서
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


/* 
    클래스 및 스마트 포인터 생성 매크로

    어떤 클래스를 선언하고
    어떤 클래스를 가리키는 스마트 포인터 alias 를 만드는 매크로
        
    ## = 두 글자를 붙인다는 뜻
*/
#define CLASS_PTR(klassName) \
class klassName; \
using klassName ## UPtr = std::unique_ptr<klassName>; \
using klassName ## Ptr = std::shared_ptr<klassName>; \
using klassName ## WPtr = std::weak_ptr<klassName>;

/* 
    텍스트 파일을 읽는 함수

    파일을 읽는 것을 성공할 수도 있지만, 실패할 수도 있다
        보통 nullptr 을 리턴함으로서 알림
        하지만 포인터 사용 => Memory Leak 위험성

    std::optional<>
        파일을 읽는 데 성공했는 지, 실패했는 지를 리턴된 객체를 통해서 알 수 있다
        성공한 경우에만 => string 값을 이용하게 할 수 있다
*/
std::optional<std::string> LoadTextFile(const std::string& filename);
// 빛 감쇠 상수 계산
glm::vec3 GetAttenuationCoeff(float distance);

#endif // __COMMON_H__