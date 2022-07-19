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

    // ī�޶� ��ü �ʱ�ȭ
    MainCam = Camera::Create();




    // Fluid �� �׸��� ���α׷�
    FluidProgram = Program::Create("./shader/fluid.vs", "./shader/fluid.fs");
    if(!FluidProgram) return false;




    // Particle ���� �������� ����ϴ� ��ǻƮ ���̴�
    FluidComputeShader = Shader::CreateFromFile("./shader/fluid.compute", GL_COMPUTE_SHADER);
    // �ش� ���̴��� ��� ���α׷� ����
    ComputeProgram = Program::Create({ FluidComputeShader });
    if(!ComputeProgram) return false;


    
    // Particle => Box �� ǥ������
    BoxMesh = Mesh::CreateBox();



    // Particle ���� �ʱ� ������ �ʱ�ȭ �Ѵ�
    InitParticles();





    // GPU ���� �����͸� ������ SSBO ���۸� ����, �ʱ�ȭ�� Particles ������ ���� => GPU �� �ִ´�
    // ���� 2�� ��� ó���� �����ϰ� �ʱ�ȭ�Ѵ�
    ParticleBufferRed 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());
    ParticleBufferBlack 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, ParticleArray.data(), sizeof(Particle), ParticleArray.size());



    
    /* 
        SSBO ���۸� �� �ε����� ���ε�

        ó�� ����
            Red Buffer   == 1 index
            Black Buffer == 2 index
     */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding_Index_One, ParticleBufferRed->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding_Index_Two, ParticleBufferBlack->Get());

    


    /* 
        ��ǻƮ ���α׷��� SSBO �� ����
        
        Input Buffer  => 1
        Output Buffer => 2

        �̷��� ����Ǿ� �ִ� ���� Compute Shader ���� ������ ���̱� ������, �����̴�
        binding = 1 or 2 �� ������ �Ǿ� �ִ�
     */
    GLuint blockIndex{};
    blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "InputBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Binding_Index_One);
    RedBufferInput = true;

    blockIndex = glGetProgramResourceIndex(ComputeProgram->Get(), GL_SHADER_STORAGE_BUFFER, "OutputBuffer");
    glShaderStorageBlockBinding(ComputeProgram->Get(), blockIndex, Binding_Index_Two);




    



    







    return true;
}




// ������ Range �� Particles �� ���� �ִ´�
void Context::InitParticles()
{
    // Particle::Fluid Range �� �յ��ϰ� ������, Particle ���� �� �� �ֵ���
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

    // ī�޶������ �Ÿ��� �������� �����Ѵ�
    std::sort(ParticleArray.begin(), ParticleArray.end(), ParticleCompare());


    // ���� ũ��� output particles �迭�� Ȯ��
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
    if (button == GLFW_MOUSE_BUTTON_RIGHT)  // ���� Ű Ŭ��
    {
        if (action == GLFW_PRESS)
        {
            // ���콺 ���� ���� ������ ���� ���콺 Ŀ�� ��ġ ����
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



    // ī�޶� ����, ��ġ ���� Ȯ��
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

    // output buffer, input buffer �Ǵ�
    BufferPtr outputBuffer = (RedBufferInput) ? ParticleBufferBlack : ParticleBufferRed;
    BufferPtr inputBuffer  = (RedBufferInput) ? ParticleBufferRed   : ParticleBufferBlack;




    // ������ �ϱ� ����, Compute Program �� �����Ų��
    ComputeProgram->Use();
        // ���� ī�޶��� ��ġ�� Uniform ���� ����
        ComputeProgram->SetUniform("CameraPos", MainCam->Position);

        
        // ���α׷��� �����Ų��
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // input ����
        // glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer->Get());
        //     Particle* particleData = (Particle*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
        //     std::sort(particleData, particleData + Particle::TotalParticleCount, ParticleCompare());
            
        //     for(int i = 0; i < 4; i++)
        //         SPDLOG_INFO("{} input data : {}", i, particleData[i].ToCamera);

        //     glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        // glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
   
        // output ����
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

    // �ϴ� ���ุ �ϰ�
    glUseProgram(0);


    



    // �������� ���� ����
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // output ���ۿ� ����� �����͸� �̿��ؼ�, Particles �� �׸���
    FluidProgram->Use();
                 
  
            // ������ �����͸� �̿��ؼ�, ī�޶󿡼� ����� �� ���� �������� ����
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
        �������� ��ģ ��, input buffer �� output buffer �� �ٲٴ� �۾��� �����Ѵ�

        ���� output buffer => ���� input buffer �� �Ǿ�� �Ѵ� => binding = 1 �� �����Ų��
        ���� input buffer => output buffer �� �ǰ� �Ѵ�

        red, black buffer �� ������ �ٲ���� ���̹Ƿ�, bool Ÿ�� ���� ������ �ش�
     */
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding_Index_One, outputBuffer->Get());
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Binding_Index_Two, inputBuffer->Get());
    // RedBufferInput = !RedBufferInput;
}