#include "program.h"

/* PUBLIC */
ProgramUPtr Program::Create(const std::vector<ShaderPtr>& shaders)
{
    auto program = ProgramUPtr(new Program());
    if (!program->Link(shaders))
        return nullptr;
    return std::move(program);
}

/* 
    Vertex Shader 와 Fragment Shader 파일 이름만 받고
    Shader -> GL shader 생성
    이후
    Program -> GL program 생성
 */
ProgramUPtr Program::Create(const std::string& vertShaderFilename, const std::string& fragShaderFilename)
{
    ShaderPtr vs = Shader::CreateFromFile(vertShaderFilename, GL_VERTEX_SHADER);
    ShaderPtr fs = Shader::CreateFromFile(fragShaderFilename, GL_FRAGMENT_SHADER);
    
    if (!vs || !fs)
        return nullptr;
    return std::move(Create({vs, fs}));
}

Program::~Program()
{
    if (m_program)
    {   
        glDeleteProgram(m_program);
    }
}

void Program::Use() const
{
    glUseProgram(m_program);
}


/* 
    shader 의 uniform 변수에 값을 전달하는 함수

    Uniform 변수를 넣을 때, 어떤 셰이더 파일에 넣는 지는 중요하지 않다
    GL program 단위로 관리되기 때문에, 프로그램 내에 셰이더가 존재하는 지를 파악해야 한다

    GL program 에 사용하려는 uniform 변수의 이름으로 접근
    해당 uniform 의 위치를 알아내고
    위치에 값을 집어 넣는다
 */
void Program::SetUniform(const std::string& name, int value) const
{
    // 주어진 이름을 가지는 Uniform 변수의 위치를 파악하고
    auto loc = glGetUniformLocation(m_program, name.c_str());
    // 해당 위치에 값을 집어 넣는다
    glUniform1i(loc, value);
}

void Program::SetUniform(const std::string& name, unsigned int value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniform1ui(loc, value);
}

void Program::SetUniform(const std::string& name, float value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniform1f(loc, value);
}

void Program::SetUniform(const std::string& name, const glm::vec2& value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniform2fv(loc, 1, glm::value_ptr(value));
}

void Program::SetUniform(const std::string& name, const glm::vec3& value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    
    // 벡터나 행렬을 넣을 때는 값을 복사하지 않고 포인터를 넘긴다 => value_ptr 사용
    glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Program::SetUniform(const std::string& name, const glm::vec4& value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniform4fv(loc, 1, glm::value_ptr(value));
}

void Program::SetUniform(const std::string& name, const glm::mat4& value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

/* PRIVATE */
bool Program::Link( const std::vector<ShaderPtr>& shaders )
{
    /* 
        glCreateProgram
            OpenGL program object 를 리소스에 할당해서 만들고
            핸들 번호, 접근할 수 있는 id 를 리턴한다
        
        만든 GL program 에 컴파일 된 셰이더들을 합칠 것이다
     */
    m_program = glCreateProgram();
    
    for (auto& shader: shaders) // 입력받은 모든 gl shaders 에 대해서
        /* 
            glAttachShader
                GL program object 에 컴파일 된 여러 개의 GL shader objects 를 붙인다
                program = shader 여러 개가 연결된 파이프라인
        */
        glAttachShader(m_program, shader->Get());


    /* 
        glLinkProgram
            컴파일 된 GL shader objects 를 링크
            shader 들을 서로 연결한다 => 온전한 gl program 생성
     */
    glLinkProgram(m_program);


    // Link 성공 여부 확인
    int success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(m_program, 1024, nullptr, infoLog);
        SPDLOG_ERROR("failed to link program: {}", infoLog);
        return false;
    }


    return true;
}