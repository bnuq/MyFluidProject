#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <imgui.h>
#include <vector>                   // CPU �� ������ ����


#include "common.h"
#include "program.h"
#include "mesh.h"
#include "shadow_map.h"


#include "../Src/Camera.h"
#include "../Src/Particle.h"


CLASS_PTR(Context)
class Context
{
public:
    // �⺻���� �Լ�
    static ContextUPtr Create();
    void Render();

    // Ű����, ���콺 �Է� ó��
    void ProcessInput(GLFWwindow* window);
    void Reshape(int, int);
    void MouseMove(double, double);
    void MouseButton(int button, int action, double x, double y);


private:
    Context() {}
    bool Init();

    // Window Information
    int m_width { WINDOW_WIDTH };
    int m_height { WINDOW_HEIGHT };
    glm::vec4 m_clearColor { glm::vec4(0.5f, 0.5f, 0.5f, 0.0f) };

    // Mouse
    glm::vec2 m_prevMousePos { glm::vec2(0.0f) };

    glm::vec2 m_giveForceMousePos { glm::vec2(0, 0) };
    bool m_giveForceMouse = false;
    glm::vec2 mouseForce { glm::vec2(0, 0) };



    // Meshes => ��� ��ü�� �ڽ��� ���·θ� ǥ��
    MeshUPtr BoxMesh;
    

    // Material => ��ü�� ���� ǥ���ϴ� �� �ؽ�ó�� ���� ���� �ʴ´�
    
    // ī�޶� ��ü
    CameraPtr MainCam;
    glm::vec3 m_cameraPos { glm::vec3(0.0f, 2.5f, 8.0f) }; // �ϴ� light ��꿡 ���
    bool m_cameraControl { false };






    /**********************************************************************************/
    
    // Programs
    ProgramUPtr DensityPressureCompute;     // 1. �е��� �з¸� ���� ���
    ProgramUPtr ForceCompute;               // 2. ������ ��¥�� ���
    ProgramUPtr MoveCompute;                // 3. ��ü �����̱�

    ProgramUPtr MakeDepthMap;               // 4. ī�޶� ���忡�� ������ ~ depth map �� �����
    ProgramUPtr DrawProgram;                // 5. ȭ�鿡 ������ ��ü�� �׸���

    // Compute Shaders
    ShaderPtr FluidDensityPressure;          // 1. �е�, �з� ��� compute shader
    ShaderPtr FluidForce;                    // 2. ��¥���� ����ϴ� compute shader
    ShaderPtr FluidMove;                     // 3. ��ü�� �������� ����ϴ� compute Shader




    // CPU ���� Core Particle �����͸� �����ϴ� �迭 => �������� �ٲ� ����Ǿ�� �ϴ� ������� ����
    std::vector<CoreParticle> CoreParticleArray{};

    // Core Particle ���� �ʱ� ��ġ ���� �ִ� �Լ�
    void Init_CoreParticles();



    // Core Particle ~ ���� ������ �Ÿ��� �������� �����ϴ� function object
    struct CoreParticle_Compare
    {
        bool operator()(const CoreParticle& p1, const CoreParticle& p2)
        {
            // ������ ��ƼŬ ~ ī�޶� ������ �Ÿ��� �������� ����
            return p1.toOrigin > p2.toOrigin;
        }
    };


    /*  
        SSBO Buffer
     */
    BufferPtr CoreParticleBuffer;                   // Core Particle �����͸� ����, �ش� Ÿ������ �����͸� �ְ� �޴´�
    BufferPtr ParticleBuffer;                       // Particle ������, GPU �������� �����ϰ� ���

    unsigned int CoreParticle_Index  = 1;
    unsigned int Particle_Index  = 2;



    // �� ���α׷����� �ʿ�� �ϴ� Uniform Variables
    // 1. Smooth Kernel
        float SmoothKernelRadius = 1.5f;
        
    // 2. Pressure Data
        struct PressureData
        {
            float gasCoeffi   = 100.0f;
            float restDensity = 0.01f;
        };
        PressureData PD;

    // 3. Viscosity
        float viscosity       = 10.0f;

    // 4. Surface
        float surfThreshold   = 0.01f;
        float surfCoeffi      = 50.0f;

    // 5. Gravity
        glm::vec3 gravityAcel = glm::vec3(0.0f, -10.0f, 0.0f);
    
    // 6. Wave Power
        float wavePower = 0.0f;

    // 7. Delta Time
        float deltaTime = 0.016f;

    // 8. Damping
        float damping   = 0.95f;

    // 9. �������� �� �����׸�Ʈ�� �ȼ� ���� ���� �� �� ����
        float offset = 0.001f;

    // 10. Alpha Offset
        float alphaOffset = 0.4f;


    // Program ���� �Լ�
    void Get_Density_Pressure();                    // 1
    void Get_Force();                               // 2
    void Get_Move();                                // 3

    void Make_Depth_Map(const glm::mat4& proj, const glm::mat4& view);  // 4
    void Draw_Particles(const glm::mat4& proj, const glm::mat4& view);  // 5



    // 4 => ī�޶� ���忡�� ���� ���� ���� �׸��� shadow map
    ShadowMapPtr depthFrameBuffer{};
    float cameraNear= 20.0f;
    float cameraFar = 200.0f;
    

    
};

#endif // __CONTEXT_H__