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
    glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);


    /* Initialize Programs */
    // 1. position, velocity, range, density, pressure ���
    FluidDensityPressure = Shader::CreateFromFile("./shader/FluidDensityPressure.compute", GL_COMPUTE_SHADER);
    DensityPressureCompute = Program::Create({ FluidDensityPressure });
    if(!DensityPressureCompute) return false;



    // 2. force, ��¥�� ���
    FluidForce = Shader::CreateFromFile("./shader/FluidForce.compute", GL_COMPUTE_SHADER);
    ForceCompute = Program::Create({ FluidForce });
    if(!ForceCompute) return false;



    // 3. Particle �� �̵���Ŵ, new position, new velocity, new toCamera
    FluidMove = Shader::CreateFromFile("./shader/FluidMove.compute", GL_COMPUTE_SHADER);
    MoveCompute = Program::Create({ FluidMove });
    if(!MoveCompute) return false;



    // 4. Shadow Map �� Particles �� ������ �׸��� => �׷����� ���̸� ���� ���
    MakeDepthMap = Program::Create("./shader/Simple/simple.vs", "./shader/Simple/simple.fs");
    if(!MakeDepthMap) return false;

        // shadow map
        depthFrameBuffer = ShadowMap::Create(1024, 1024);



    // 5. ���� ������ ������, depth map �� �����ؼ� ��Ÿ�� �ȼ��� ã�´�
    DrawProgram = Program::Create("./shader/Fluid/fluid.vs", "./shader/Fluid/fluid.fs");
    if(!DrawProgram) return false;






    // ī�޶� ��ü �ʱ�ȭ
    MainCam = Camera::Create();
    
    // Particle => Box �� ǥ������
    BoxMesh = Mesh::CreateBox();


    


    // Core Particles �� �ʱ� ������ �ʱ�ȭ �Ѵ�
    // �׸��� Core Particles �� ����
    Init_CoreParticles();





    // GPU ���� �����͸� ������ SSBO ���۸� ���� �� �Ҵ�
    // Core Particle => �����͸� �����ؼ� GPU �� �ִ´�
    // ���ĵ� �����͸� �ִ´�
    CoreParticleBuffer
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, CoreParticleArray.data(), sizeof(CoreParticle), CoreParticleArray.size());


    // Particle      => ������ �����ʹ� ����, GPU ���� ������ �Ҵ��� ���´�
    ParticleBuffer 
        = Buffer::CreateWithData(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NULL, sizeof(Particle), CoreParticleArray.size());


    
    /* SSBO ���۸� �� �ε����� ���ε� */
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CoreParticle_Index, CoreParticleBuffer->Get());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Particle_Index,     ParticleBuffer->Get());

    
    
    /* ��ǻƮ ���α׷��� SSBO �� ���� */
    GLuint blockIndex{};

    // 1. Density, Pressure
        blockIndex = glGetProgramResourceIndex(DensityPressureCompute->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(DensityPressureCompute->Get(), blockIndex, CoreParticle_Index);
        blockIndex = glGetProgramResourceIndex(DensityPressureCompute->Get(), GL_SHADER_STORAGE_BUFFER, "ParticleBuffer");
        glShaderStorageBlockBinding(DensityPressureCompute->Get(), blockIndex, Particle_Index);


    // 2. Force
        blockIndex = glGetProgramResourceIndex(ForceCompute->Get(), GL_SHADER_STORAGE_BUFFER, "ParticleBuffer");
        glShaderStorageBlockBinding(ForceCompute->Get(), blockIndex, Particle_Index);
    

    // 3. Move
        blockIndex = glGetProgramResourceIndex(MoveCompute->Get(), GL_SHADER_STORAGE_BUFFER, "ParticleBuffer");
        glShaderStorageBlockBinding(MoveCompute->Get(), blockIndex, Particle_Index);
        blockIndex = glGetProgramResourceIndex(MoveCompute->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(MoveCompute->Get(), blockIndex, CoreParticle_Index);


    // 4. Make Depth Map    => Core Particle ���
        blockIndex = glGetProgramResourceIndex(MakeDepthMap->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(MakeDepthMap->Get(), blockIndex, CoreParticle_Index);


    // 5. Rendering         => Core Particle ���
        blockIndex = glGetProgramResourceIndex(DrawProgram->Get(), GL_SHADER_STORAGE_BUFFER, "CoreParticleBuffer");
        glShaderStorageBlockBinding(DrawProgram->Get(), blockIndex, CoreParticle_Index);



    return true;
}




// ������ Range �� Particles �� ���� �ִ´�
void Context::Init_CoreParticles()
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
                // Core Particle Initialize
                CoreParticleArray.push_back
                ( 
                    CoreParticle
                    (
                        glm::vec3(xStride * xCount, yStride * yCount, zStride * zCount)
                    )
                );
            }
        }
    }


    // ī�޶������ �Ÿ��� �������� �����Ѵ� => �ָ� �ִ� ���� ������ �´�
    // ó������ �ƿ� ������ �ϰ� ����
    std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_Compare());
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
}
void Context::MouseMove(double x, double y)
{
    if(m_cameraControl)
    {
        auto pos = glm::vec2((float)x, (float)y);
        auto deltaPos = pos - m_prevMousePos;

        MainCam->Rotate(deltaPos);
        m_prevMousePos = pos;
    }

    if(m_giveForceMouse)
    {
        auto pos = glm::vec2((float)x, (float)y);
        auto deltaPos = pos - m_giveForceMousePos;

        mouseForce += deltaPos;
        m_giveForceMousePos = pos;
    }
}
void Context::MouseButton(int button, int action, double x, double y)
{
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)  // �߰� Ű Ŭ��
    {
        if (action == GLFW_PRESS)
        {
            // ���콺 ���� ���� ������ ���� ���콺 Ŀ�� ��ġ ����
            m_giveForceMousePos = glm::vec2((float)x, (float)y);
            m_giveForceMouse = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_giveForceMouse = false;
        }
    }


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


    if(ImGui::Begin("Smooth Kernel"))
    {
        // 1. Smmoth Kernel
            ImGui::DragFloat("Smooth Kernel Radius", &SmoothKernelRadius, 0.01f, 0.01f);

        // Particle Data
            ImGui::DragFloat("particle mass", &Particle::ParticleMass, 0.01f, 0.0f);

        // 2. Pressure
            ImGui::DragFloat("Gas Coeffi", &PD.gasCoeffi, 1.0f, 0);
            ImGui::DragFloat("Gas RestDensity", &PD.restDensity, 0.001f, 0);

        // 3. Viscosity
            ImGui::DragFloat("viscosity", &viscosity);

        // 4. Surface
            ImGui::DragFloat("surfThreshold", &surfThreshold, 0.001f, 0.0f);
            ImGui::DragFloat("surfCoeffi", &surfCoeffi, 0.01f, 0.0f);

        // 5. Gravity
            ImGui::DragFloat4("gravityAcel", glm::value_ptr(gravityAcel));

        // 6. Wave Poser
            ImGui::DragFloat("wavePower", &wavePower, 1.0f, 0);

        // 7. Delta Time
            ImGui::DragFloat("deltaTime", &deltaTime);

        // 8. Collision
            ImGui::DragFloat("damping", &damping);

        // 9. Renderings
            ImGui::DragFloat("offset", &offset, 0.001f, 0);

        // 10. Alpha Offset
            ImGui::DragFloat("alphaOffset", &alphaOffset, 0.01f, 0.0f, 1.0f);
    }

    // ���� �� Ȯ��
    ImGui::Image((ImTextureID)depthFrameBuffer->GetShadowMap()->Get(), ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::DragFloat("cameraNear", &cameraNear, 1.0f, 0);
    ImGui::DragFloat("cameraFar", &cameraFar, 1.0f, 0);



    ImGui::DragFloat3("Camera Position", glm::value_ptr(MainCam->Position));

    ImGui::End();





    // ī�޶� ����, ��ġ ���� Ȯ��
    auto projection = glm::perspective
    (
        glm::radians(45.0f),
        (float)m_width / (float)m_height,
        cameraNear, cameraFar
    );

    auto view = glm::lookAt
    (
        MainCam->Position,
        MainCam->Position + MainCam->DirVec,
        MainCam->UpVec
    );




    // ���α׷� ����
    Get_Density_Pressure();                             // 1. ���� ������ �е��� �з� ������ ���ϰ�
    Get_Force();                                        // 2. �� ���ڵ��� ��¥���� ���Ѵ�

    Get_Move();                                         // 3. ��¥���� ���� �̵� -> ���� ��ġ�� �ӵ��� ����

    Make_Depth_Map(projection, view);                   // 4. �ϴ� Depth map �� ������ => Depth Map �� �����ϰ�
    Draw_Particles(projection, view);                   // 5. ���������� visible particle �鸸 ������
}




// Core Particle �� input ���� ��� => ������ ���⼭ ���� �����ͼ� ����Ѵ�
// Particle �� output => ������ �̰��� �����Ѵ�
void Context::Get_Density_Pressure()
{
    DensityPressureCompute->Use();
        
        // Smmoth Kernel
            DensityPressureCompute->SetUniform("h", SmoothKernelRadius);

        // Particle Data
            DensityPressureCompute->SetUniform("particleMass", Particle::ParticleMass);
            DensityPressureCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);

        // Pressure Data
            DensityPressureCompute->SetUniform("gasCoeffi", PD.gasCoeffi);
            DensityPressureCompute->SetUniform("restDensity", PD.restDensity);

        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    glUseProgram(0);
}



// ����� �Ϸ�� Particle �����͸� input ���� ���
// ä������ ���� Particle �����͸� output ���� ä���
void Context::Get_Force()
{
    ForceCompute->Use();

        // Smmoth Kernel
            ForceCompute->SetUniform("h", SmoothKernelRadius);

        // Particle Data
            ForceCompute->SetUniform("particleMass", Particle::ParticleMass);
            ForceCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);
            ForceCompute->SetUniform("xRange", Particle::FluidRange.x);
            ForceCompute->SetUniform("yRange", Particle::FluidRange.y);
        
        // Viscosity
            ForceCompute->SetUniform("viscosity", viscosity);

        // Surface
            ForceCompute->SetUniform("surfThreshold", surfThreshold);
            ForceCompute->SetUniform("surfCoeffi", surfCoeffi);

        // Gravity
            ForceCompute->SetUniform("gravityAcel", gravityAcel);

        // Mouse Force
        // ���콺 ��Ŭ���� ���µ�, ���� ���� ��, ���� �ѱ�� �� �ʱ�ȭ
            if(!m_giveForceMouse && (glm::length(mouseForce) != 0))
            {
                ForceCompute->SetUniform("mouseForce", mouseForce);
                mouseForce = glm::vec2(0);
            }
            else
                ForceCompute->SetUniform("mouseForce", glm::vec2(0));

        // Wave Force
            ForceCompute->SetUniform("wavePower", wavePower);

        // Total Time
        ForceCompute->SetUniform("time", (float)glfwGetTime());


        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    glUseProgram(0);
}


void Context::Get_Move()
{
    MoveCompute->Use();

        // Particle Data
            MoveCompute->SetUniform("TotalParticleCount", Particle::TotalParticleCount);
            MoveCompute->SetUniform("LimitRange", Particle::FluidRange);

        // Delta Time
            MoveCompute->SetUniform("deltaTime", deltaTime);

        // Collision
            MoveCompute->SetUniform("damping", damping);


        glDispatchCompute(Particle::GroupNum, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        
        // ���Ӱ� ����� Core Particle ������ => ī�޶���� �Ÿ��� ���缭 ������ �ٽ� ���ش�
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, CoreParticleBuffer->Get());

            // ���� ���, Output �� CPU �� �о����
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());

            // Camera ������ �Ÿ��� �������� sort
            std::sort(CoreParticleArray.begin(), CoreParticleArray.end(), CoreParticle_Compare());

            // �ٽ� �־��ش� => ���� ������ �� input ���� ����Ѵ�
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(CoreParticle) * CoreParticleArray.size(), CoreParticleArray.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(0);
}




void Context::Make_Depth_Map(const glm::mat4& projection, const glm::mat4& view)
{
    // ȭ�鿡 �׸��� �ʰ�, shadow map �� �׷���, ī�޶󿡼� ���� �� �� �ȼ� depth ���� �ؽ�ó�� �����Ѵ�
    depthFrameBuffer->Bind();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport  // view port ����
        (
            0, 
            0,
            depthFrameBuffer->GetShadowMap()->GetWidth(),
            depthFrameBuffer->GetShadowMap()->GetHeight()
        );

        MakeDepthMap->Use();

            // vertex shader
                MakeDepthMap->SetUniform("transform", projection * view);

            BoxMesh->GPUInstancingDraw(MakeDepthMap.get(), Particle::TotalParticleCount);

        glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, m_width, m_height);
}





void Context::Draw_Particles(const glm::mat4& projection, const glm::mat4& view)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Particle Buffer �� ����� �����͸� �̿��ؼ�, Particles �� �׸���
    DrawProgram->Use();
        
        // vertex shader
            DrawProgram->SetUniform("transform", projection * view);
            DrawProgram->SetUniform("CameraPos", MainCam->Position);

        // fragment shader
            glActiveTexture(GL_TEXTURE0);
            depthFrameBuffer->GetShadowMap()->Bind();
            DrawProgram->SetUniform("depthMap", 0);

            DrawProgram->SetUniform("offset", offset);
            DrawProgram->SetUniform("alphaOffset", alphaOffset);           
            DrawProgram->SetUniform("CameraDir", MainCam->DirVec);
            
        BoxMesh->GPUInstancingDraw(DrawProgram.get(), Particle::TotalParticleCount);

    glUseProgram(0);

    glDisable(GL_BLEND);
}