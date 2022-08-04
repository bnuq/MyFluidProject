#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "common.h"
#include "shader.h"

/* 
    Program 클래스
        GL program object 를 관리
    
    GL program
        여러 개의 컴파일 된 shader 들을 합쳐놓은 것
        programmable, 내가 수정할 수 있는 셰이더들을 모아놓았다
        어떤 셰이더를 사용할 지 결정할 수 있다
        셰이더들에 들어가는 Uniform 관리
 */
CLASS_PTR(Program)
class Program
{
public:
    static ProgramUPtr Create(const std::vector<ShaderPtr>& shaders);
    static ProgramUPtr Create(const std::string& vertShaderFilename, const std::string& fragShaderFilename);
    ~Program();
    /* 
        관리하고 있는 GL program 을 사용하고 싶을 때 사용
            GL program 이라는 것은, 컴파일 된 셰이더들을 모아놓은 것
            즉, 내가 작성한 어떤 셰이더들을 사용하고 싶을 때 호출
     */    
    void Use() const;

    // Return GL program handle id
    uint32_t Get() const { return m_program; }
    /* 
        shader 에 uniform 변수를 넘기는 함수들
        넘기는 Uniform 변수에 맞게 function overloading

        Uniform 변수의 이름 문자열을 통해서 접근한다
     */
    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, unsigned int value) const;
    void SetUniform(const std::string& name, float value) const;

    void SetUniform(const std::string& name, const glm::vec2& value) const;
    void SetUniform(const std::string& name, const glm::vec3& value) const;
    void SetUniform(const std::string& name, const glm::vec4& value) const;

    void SetUniform(const std::string& name, const glm::mat4& value) const;

private:
    Program() {}
    // compile 한 GL shader 들을 Link => 온전한 GL program 을 만든다
    bool Link(const std::vector<ShaderPtr>& shaders);

    // Program Object id
    uint32_t m_program { 0 };
};

#endif // __PROGRAM_H__