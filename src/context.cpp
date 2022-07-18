#include "context.h"
#include <vector>
#include <string>


ContextUPtr Context::Create()                                  
{
    auto context = ContextUPtr(new Context());
    if (!context->Init())
        return nullptr;
    return std::move(context);
}


bool Context::Init()
{
    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

    // 카메라 객체 초기화
    MainCam = Camera::Create();
    if(MainCam == nullptr)
        SPDLOG_INFO("NULLPTR camera");


    SPDLOG_INFO("size of float is {}", sizeof(float));
    SPDLOG_INFO("size of Particle is {} and count is {}", sizeof(Particle), (sizeof(Particle) / sizeof(float)));

    // Fluid 를 그리는 프로그램
    FluidProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!FluidProgram) return false;


    // Particle 들의 움직임을 계산하는 컴퓨트 셰이더
    FluidComputeShader = Shader::CreateFromFile("./shader/fluid.compute", GL_COMPUTE_SHADER);
    // 해당 셰이더를 담는 프로그램 생성
    ComputeProgram = Program::Create({FluidComputeShader});
    if(!ComputeProgram) return false;


    
    // Particle => Box 로 표현하자
    BoxMesh = Mesh::CreateBox();


    // Particle 들의 초기 정보를 초기화 한다
    InitParticles();


    // GPU 에서 데이터를 저장할 SSBO 버퍼를 생성, 초기화한 Particles 정보를 복사 => GPU 에 넣는다
    ParticleBuffer = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());
    
    // SSBO 버퍼 바인딩
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Particle_Binding_Index, ParticleBuffer->Get());

    
    // 컴퓨트 프로그램과 SSBO 를 연결
    auto blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "ParticleBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Particle_Binding_Index);


    return true;
}




// 정해진 Range 에 Particles 를 집어 넣는다
void Context::InitParticles()
{
    // Particle::Fluid Range 를 균등하게 나눈다, Particle 들이 들어갈 수 있도록
    float xStride = Particle::FluidRange.x / (Particle::ParticleCount.x + 1);
    float yStride = Particle::FluidRange.y / (Particle::ParticleCount.y + 1);
    float zStride = Particle::FluidRange.z / (Particle::ParticleCount.z + 1);


    for(unsigned int xCount = 1; xCount <= Particle::ParticleCount.x; xCount++)
    {
        for(unsigned int yCount = 1; yCount <= Particle::ParticleCount.y; yCount++)
        {
            for(unsigned int zCount = 1; zCount <= Particle::ParticleCount.z; zCount++)
            {
                ParticleArray.push_back( Particle(glm::vec3(xStride * xCount, yStride * yCount, zStride * zCount)) );
            }
        }
    }



    for(auto p : ParticleArray)
    {
        SPDLOG_INFO("{}, {}, {}", p.Position.x, p.Position.y, p.Position.z);
    }
}




void Context::ProcessInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_W);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_S);

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_A);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_D);

    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_Q);
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        MainCam->Move(GLFW_KEY_E);

}
void Context::Reshape(int width, int height)
{
    m_width = width;
    m_height = height;

    glViewport(0, 0, m_width, m_height);

    //m_framebuffer = Framebuffer::Create(Texture::Create(m_width, m_height, GL_RGBA));
}
void Context::MouseMove(double x, double y)
{
    if(!m_cameraControl) return;


    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;

    MainCam->Rotate(deltaPos);
    m_prevMousePos = pos;
}
void Context::MouseButton(int button, int action, double x, double y)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)  // 우측 키 클릭
    {
        if (action == GLFW_PRESS)
        {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_cameraControl = false;
        }
    }
}





void Context::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // 카메라 설정, 위치 등을 확정
    auto projection = glm::perspective
    (
        glm::radians(45.0f),
        (float)m_width / (float)m_height,
        0.01f, 500.0f
    );

    auto view = glm::lookAt
    (
        //glm::vec3(0, 0, -5.0f),
        MainCam->Position,
        //glm::vec3(0.0f, 0.0f, 0.0f),
        MainCam->Position + MainCam->DirVec,
        MainCam->UpVec
    );

    // Compute Program 을 실행 => 각 Particle 의 데이터를 계산한다
        // Particles 의 위치를 저장하는 배열을 이용해서 계산, 접근
        // 계산 결과 데이터 => Particles 의 위치가 저장된 3-4 차 배열에 담겨서 나온다

    // 또 다른 Compute Program 을 실행
    // 각 위치에서 스레드가 돌아 ~ 표면이 되는 Particle 인덱스만 따로 알아내서
    // 표면이 되는 Particle 인덱스에만 true 를 저장하는 버퍼를 리턴한다
    // 전체 그리는 개수도 파악한다

    // 그려지는 인덱스를 저장한 버퍼 => 순차대로 돌면서, 카메라 - Particle 거리를 계산
    // Map 에 저장한다

    // 마지막으로 Map 에 저장된 Particle 을 차례대로 돌면서
    // 카메라에서 멀리 떨어진 Particle 부터 그린다


    FluidProgram->Use();
        for(auto p : ParticleArray)
        {
            auto modelTransform = glm::translate(glm::mat4(1.0f), p.Position);
            auto transform = projection * view * modelTransform;

            FluidProgram->SetUniform("transform", transform);

            BoxMesh->Draw(FluidProgram.get());
        }
    glUseProgram(0);

    
}