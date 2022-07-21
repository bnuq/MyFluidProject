#include "context.h"

#include <vector>
#include <string>
#include <algorithm>


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


    // Initialize Programs
    // Fluid 를 그리는 프로그램
    DrawProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!DrawProgram) return false;

    // Particle 들의 움직임을 계산하는 컴퓨트 셰이더
    FluidComputeShader = Shader::CreateFromFile("./shader/fluid.compute", GL_COMPUTE_SHADER);
    // 해당 셰이더를 담는 프로그램 생성
    ComputeProgram = Program::Create({ FluidComputeShader });
    if(!ComputeProgram) return false;

    // Particle 를 움직이는 셰이더
    FluidMoveShader = Shader::CreateFromFile("./shader/fluidMove.compute", GL_COMPUTE_SHADER);
    // 해당 셰이더를 담는 프로그램 생성
    MoveProgram = Program::Create({ FluidMoveShader });
    if(!MoveProgram) return false;



    // 카메라 객체 초기화
    MainCam = Camera::Create();
    
    // Particle => Box 로 표현하자
    BoxMesh = Mesh::CreateBox();



    // Particle 들의 초기 정보를 초기화 한다
    InitParticles();


    // GPU 에서 데이터를 저장할 SSBO 버퍼를 생성, 초기화한 Particles 정보를 복사 => GPU 에 넣는다
    // 버퍼 2개 모두 처음에 동일하게 초기화한다
    InputBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());
    OutputBuffer
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());

    
    /* 
        SSBO 버퍼를 각 인덱스에 바인딩

        Input Buffer   == 1 index
        Output Buffer  == 2 index
     */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Input_Index, InputBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Output_Index, OutputBuffer->Get());



    /* 
        컴퓨트 프로그램과 SSBO 를 연결
        
        Input Buffer  => 1
        Output Buffer => 2

        이렇게 연결되어 있는 것은 Compute Shader 에서 지정한 것이기 때문에, 고정이다
        binding = 1 or 2 로 고정이 되어 있다
     */
    GLuint blockIndex{};

    blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "InputBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Input_Index);
    blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Output_Index);

    blockIndex = glGetProgramResourceIndex(MoveProgram->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(MoveProgram->Get(), blockIndex, Output_Index);

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
                ParticleArray.push_back( 
                    Particle(glm::vec4(xStride * xCount, yStride * yCount, zStride * zCount, 1.0f), MainCam->Position)
                );
            }
        }
    }

    // 카메라까지의 거리를 기준으로 정렬한다
    std::sort(ParticleArray.begin(), ParticleArray.end(), ParticleCompare());
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

   


    // 렌더링 하기 전에, Compute Program 을 실행 => 계산하고
    ComputeProgram->Use();
        /* Uniform Variables Setting */
        // Particles 에 대한 정보
        ComputeProgram->SetUniform("particlelMass", Particle::ParticleMass);
        ComputeProgram->SetUniform("particlelCount", Particle::TotalParticleCount);

        // Smooth Kernel 에 대한 정보
        ComputeProgram->SetUniform("h", 1.0f);
        ComputeProgram->SetUniform("hSquare", 1.0f);

        // 밀도=>압력 계산에 필요한 정보
        ComputeProgram->SetUniform("gasCoeffi", 1.0f);
        ComputeProgram->SetUniform("restDensity", 1.0f);


        // 프로그램을 실행시킨다
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // 일단 실행만 하고
    glUseProgram(0);



    // 계산 결과를 이용해서 움직이게 한다
    MoveProgram->Use();
        /* Uniform Variable 넣기 */
        MoveProgram->SetUniform("deltaTime", 0.0001f);
        MoveProgram->SetUniform("LimitRange", Particle::FluidRange);
        MoveProgram->SetUniform("damping", 0.1f);
        
        // 현재 카메라의 위치를 Uniform 으로 설정
        // Particle 이 최종적으로 움직인 위치에서 카메라까지의 거리를 구해야 한다
        MoveProgram->SetUniform("CameraPos", MainCam->Position);


        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, OutputBuffer->Get());
            // 연산 결과, Output 을 CPU 로 읽어오고
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());
            // Camera 까지의 거리를 기준으로 sort
            std::sort(ParticleArray.begin(), ParticleArray.end(), ParticleCompare());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, InputBuffer->Get());
            // sort 한 결과를 input buffer 에 넣는다
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * ParticleArray.size(), ParticleArray.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
    
    /* 
        Output 버퍼에 들어있는 데이터들에서 한번더 정렬을 해야할 듯?
        그냥 루프를 도는 횟수를 줄이는 게 가장 좋다, 배열을 줄이는 게 가장 좋다

        표면 Particle 의 개수를 미리 알고 있다고 가정 -
            Compute Shader 에서 계산할 수 있다고 생각
            Draw Call 하는 개수가 되는 것

        이미 정렬된 Particle Array = 멀리 있는 것 => 가까이 있는 것, 순서로 들어있다
            뒤쪽에서 표면 Particle 의 개수 ~ 끝 까지 Draw Call 을 하면
            그정도 카메라에서 가까운 Particle 들이 출력되는 데
                가까이 있는 데 표면은 아닌 데이터가 있을 거 아니냐
                그런건 거르고 싶다

            표면이 아니면 무조건 뒤로 빼고
            표면이면서
                카메라로부터 거리가 먼 것을 앞으로 빼는 정렬
            [    표면    ][비표면]
             먼 -> 가까움
             표면 개수만큼 존재

            <------------> 딱 이 정도만 Draw Call 하면 되지 않을까?
     */




    // 렌더링을 위한 설정
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // output 버퍼에 저장된 데이터를 이용해서, Particles 를 그린다
    DrawProgram->Use();
  
            // 정렬한 데이터를 이용해서, 카메라에서 가까운 것 부터 렌더링을 진행
            for(int i = 0; i < Particle::TotalParticleCount; i++)
            {
                auto ParticlePos = glm::vec3(ParticleArray[i].Position.x, ParticleArray[i].Position.y, ParticleArray[i].Position.z);

                auto modelTransform = glm::translate(glm::mat4(1.0f), ParticlePos);
                auto transform = projection * view * modelTransform;

                DrawProgram->SetUniform("transform", transform);
                DrawProgram->SetUniform("ViewVec", glm::normalize(MainCam->Position - ParticlePos));


                BoxMesh->Draw(DrawProgram.get());

            }

    glUseProgram(0);

    glDisable(GL_BLEND);
}