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

    // 카메라 객체 초기화
    MainCam = Camera::Create();




    // Fluid 를 그리는 프로그램
    FluidProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!FluidProgram) return false;




    // Particle 들의 움직임을 계산하는 컴퓨트 셰이더
    FluidComputeShader = Shader::CreateFromFile("./shader/fluid.compute", GL_COMPUTE_SHADER);
    // 해당 셰이더를 담는 프로그램 생성
    ComputeProgram = Program::Create({ FluidComputeShader });
    if(!ComputeProgram) return false;


    
    // Particle => Box 로 표현하자
    BoxMesh = Mesh::CreateBox();



    // Particle 들의 초기 정보를 초기화 한다
    InitParticles();





    // GPU 에서 데이터를 저장할 SSBO 버퍼를 생성, 초기화한 Particles 정보를 복사 => GPU 에 넣는다
    // 버퍼 2개 모두 처음에 동일하게 초기화한다
    ParticleBufferRed 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());
    ParticleBufferBlack 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());



    
    /* 
        SSBO 버퍼를 각 인덱스에 바인딩

        처음 세팅
            Red Buffer   == 1 index
            Black Buffer == 2 index
     */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding_Index_One, ParticleBufferRed->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding_Index_Two, ParticleBufferBlack->Get());

    


    /* 
        컴퓨트 프로그램과 SSBO 를 연결
        
        Input Buffer  => 1
        Output Buffer => 2

        이렇게 연결되어 있는 것은 Compute Shader 에서 지정한 것이기 때문에, 고정이다
        binding = 1 or 2 로 고정이 되어 있다
     */
    GLuint blockIndex{};
    blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "InputBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Binding_Index_One);
    RedBufferInput = true;

    blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Binding_Index_Two);




    



    







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


    // 같은 크기로 output particles 배열을 확보
    OutputParticles.resize(Particle::TotalParticleCount);

    SPDLOG_INFO("Output Particle Size is {}", OutputParticles.size());
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

    // output buffer, input buffer 판단
    BufferPtr outputBuffer = (RedBufferInput) ? ParticleBufferBlack : ParticleBufferRed;
    BufferPtr inputBuffer  = (RedBufferInput) ? ParticleBufferRed   : ParticleBufferBlack;




    // 렌더링 하기 전에, Compute Program 을 실행시킨다
    ComputeProgram->Use();
        // 현재 카메라의 위치를 Uniform 으로 설정
        ComputeProgram->SetUniform("CameraPos", MainCam->Position);

        
        // 프로그램을 실행시킨다
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // input 점검
        // glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer->Get());
        //     Particle* particleData = (Particle*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
        //     std::sort(particleData, particleData + Particle::TotalParticleCount, ParticleCompare());
            
        //     for(int i = 0; i < 4; i++)
        //         SPDLOG_INFO("{} input data : {}", i, particleData[i].ToCamera);

        //     glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        // glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
   
        // output 점검
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer->Get());
            // auto particleData = (Particle*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
            
            // //std::sort(particleData, particleData + Particle::TotalParticleCount, ParticleCompare());
            // memcpy(particleData, OutputParticles.data(), sizeof(Particle) * OutputParticles.size());
            // //std::sort(OutputParticles.begin(), OutputParticles.end(), ParticleCompare());
            
            // for(auto i : OutputParticles)
            //     SPDLOG_INFO("{}", i.ToCamera);
            // SPDLOG_INFO("END");

        
            // glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * Particle::TotalParticleCount, OutputParticles.data());
            std::sort(OutputParticles.begin(), OutputParticles.end(), ParticleCompare());
            //glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Particle) * Particle::TotalParticleCount, OutputParticles.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 일단 실행만 하고
    glUseProgram(0);


    



    // 렌더링을 위한 설정
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // output 버퍼에 저장된 데이터를 이용해서, Particles 를 그린다
    FluidProgram->Use();
                 
  
            // 정렬한 데이터를 이용해서, 카메라에서 가까운 것 부터 렌더링을 진행
            for(int i = 0; i < Particle::TotalParticleCount; i++)
            {
                auto modelTransform = glm::translate(glm::mat4(1.0f), 
                                        glm::vec3(OutputParticles[i].Position.x, OutputParticles[i].Position.y, OutputParticles[i].Position.z)
                                    );
                auto transform = projection * view * modelTransform;

                FluidProgram->SetUniform("transform", transform);

                BoxMesh->Draw(FluidProgram.get());
            }

    glUseProgram(0);

    glDisable(GL_BLEND);

    

    /* 
        렌더링을 마친 후, input buffer 와 output buffer 를 바꾸는 작업을 진행한다

        현재 output buffer => 다음 input buffer 가 되어야 한다 => binding = 1 에 연결시킨다
        현재 input buffer => output buffer 가 되게 한다

        red, black buffer 의 역할이 바뀌었을 것이므로, bool 타입 값을 뒤집어 준다
     */
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding_Index_One, outputBuffer->Get());
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding_Index_Two, inputBuffer->Get());
    // RedBufferInput = !RedBufferInput;
}