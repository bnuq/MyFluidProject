#include "shader.h"

/* PUBLIC */
ShaderUPtr Shader::CreateFromFile(const std::string& filename, GLenum shaderType)
{
    auto shader = std::unique_ptr<Shader>(new Shader());
    if (!shader->LoadFile(filename, shaderType))
        return nullptr;
    return std::move(shader);
}

Shader::~Shader()
{
    if(m_shader)
    {
        glDeleteShader(m_shader);
    }
}

/* PRIVATE */
bool Shader::LoadFile(const std::string& filename, GLenum shaderType)
{
    auto result = LoadTextFile(filename);
    // optional::has_value() => 값이 존재하는 지, 아닌 지를 판단하는 메서드
    if (!result.has_value()) // 값이 없다 == 파일을 열지 못한 경우
        return false;
    // optional::value() => optional 이 감싸고 있는 값 리턴
    // Text 파일 내 작성된 코드
    auto& code = result.value();
    /*
        string::c_str() => string 이 저장하고 있는 문자열 주소 리턴
        string => c-style string 변환
        GLshader 에는 c-style 형태로 저장된다
     */
    const char* codePtr = code.c_str();             // file 내 코드 문자열 주소
    int32_t codeLength = (int32_t)code.length();    // file 내 코드 길이
    /* 
        gl shader obj 를 생성

        glCreateShader
            GL shader object 를 생성
            핸들 번호를 리턴
            
        shader type 을 통해서 어떤 종류의 셰이더를 만드는 것인지를 알린다
            Vertex Shader
            Fragment Shader
            Compute Shader 
            등등 GLshader 의 종류를 결정하고, 생성한다 
     */
    m_shader = glCreateShader(shaderType);
    /* 
        gl shader 안에 소스코드를 넣는다

        glShaderSource
            GL shader object 에 shader code 를 집어 넣는다
            1개 이상의 코드를 넣을 수 있다, 개수 지정
            코드와 코드 길이 전달

        <참고>
            const 는 왼쪽에 적용된다
            왼쪽에 아무것도 없다면 오른쪽에 적용된다
            따라서 const GLchar* const* == const GLchar**
     */
    glShaderSource(m_shader, 1, (const GLchar**)&codePtr, &codeLength);
    // GLshader compile, 셰이더 코드를 넣은 다음, 바로 컴파일을 진행한다
    glCompileShader(m_shader);
   
    /*
        glGetShader|iv
            shader 에 있는 정보를 가져온다
            어떤 정보를 알아내고 싶은 지를 전달
            iv = integer pointer parameter
            넘긴 정수 포인터에 정보를 저장해서 전달한다
     */
    int success = 0;
    glGetShaderiv(m_shader, GL_COMPILE_STATUS, &success);
    if (!success) // 컴파일 에러 시, 에러 내용 확인
    {
        char infoLog[1024];

        // shader object 의 에러 내용을 가져온다
        glGetShaderInfoLog(m_shader, 1024, nullptr, infoLog);

        // speed log 사용
        SPDLOG_ERROR("failed to compile shader: \"{}\"", filename);
        SPDLOG_ERROR("reason: {}", infoLog);
        return false;
    }


    SPDLOG_INFO("{} shader make, id = {}", filename, m_shader);
    return true;
}