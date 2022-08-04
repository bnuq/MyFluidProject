#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <imgui.h>
#include <vector>
#include <map>
#include <deque>


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

    // ��, ����Ʈ
    struct Light
    {
        glm::vec3 position { glm::vec3(1.0f, 4.0f, 4.0f) };

        glm::vec3 direction{ glm::vec3(-1.0f, -1.0f, -1.0f) };
        glm::vec2 cutoff { glm::vec2(120.0f, 5.0f) };
        
        glm::vec3 ambient { glm::vec3(0.1f, 0.1f, 0.1f) };
        glm::vec3 diffuse { glm::vec3(0.5f, 0.5f, 0.5f) };
        glm::vec3 specular { glm::vec3(1.0f, 1.0f, 1.0f) };

        float distance { 128.0f };
    };
    Light m_light;





    /**********************************************************************************/
    
    // Programs
    ProgramUPtr DensityPressureCompute;     // �е��� �з¸� ���� ���
    ProgramUPtr ForceCompute;               // ������ ��¥�� ���
    ProgramUPtr MoveCompute;                // ��ü �����̱�
    ProgramUPtr VisibleCompute;             // ���̴� �� �����ϱ�

    ProgramUPtr SimpelProgram;              // �ܻ����� ĥ�ϴ� simple program
    ProgramUPtr DrawProgram;                // ��ü �׸���



    // Compute Shaders
    ShaderPtr FluidDensityPressure;          // �е�, �з� ��� compute shader
    ShaderPtr FluidForce;                    // ��¥���� ����ϴ� compute shader
    ShaderPtr FluidMove;                     // ��ü�� �������� ����ϴ� compute Shader
    ShaderPtr FluidVisible;                  // ��ƼŬ ��, ���� ���̴� ���� ã�� compute shader




    // CPU ���� Core Particle �����͸� �����ϴ� �迭 => �������� �ٲ� ����Ǿ�� �ϴ� ������� ����
    std::vector<CoreParticle> CoreParticleArray{};


    // Core Particle ���� �ʱ� ��ġ ���� �ִ� �Լ�
    void Init_CoreParticles();


    // Core Particle ~ ī�޶� ������ �Ÿ��� �����ϴ� function object
    struct CoreParticle_toCamera_Compare
    {
        bool operator()(const CoreParticle& p1, const CoreParticle& p2)
        {
            // ������ ��ƼŬ ~ ī�޶� ������ �Ÿ��� �������� ����
            return p1.toCamera > p2.toCamera;
        }
    };


    // ��ƼŬ���� �������ϱ� ����, visible �����͸� �̿��ؼ� �����ϴ� function object
    struct CoreParticle_visible_Compare
    {
        bool operator()(const CoreParticle& p1, const CoreParticle& p2)
        {
            // �Ѵ� ���̴� ���
            if(p1.visible != 0 && p2.visible != 0)
                // ī�޶� �Ÿ��� �̿�, �Ÿ��� �� �� ���� ������ �´�
                return p1.toCamera > p2.toCamera;
            else
                // �� �̿ܿ��� ���̴� ���� ������ ���� �����Ѵ�
                return p1.visible > p2.visible;
        }
    };
    

    // visible core particles �� ����
    unsigned int visibleCount = 0;


    /*  
        SSBO Buffer
     */
    BufferPtr CoreParticleBuffer;       // Core Particle �����͸� ����, �ش� Ÿ������ �����͸� �ְ� �޴´�
    BufferPtr ParticleBuffer;           // Particle ������, GPU �������� �����ϰ� ���
    BufferPtr CountBuffer;              // visible count ���� �о���� ����


    unsigned int CoreParticle_Index  = 1;
    unsigned int Particle_Index  = 2;
    unsigned int Count_Index = 5;



    // �� ���α׷����� �ʿ�� �ϴ� Uniform Variables
        // 1. Smooth Kernel
        float SmoothKernelRadius = 1.0f;
        //float SmoothKernelRadius = 10.0f;
        
        // 2. Gas
        struct Gas
        {
            float gasCoeffi   = 0.001f;
            //float restDensity = 1200.0f;
            float restDensity = 50.0f;
        };
        Gas gas;

        // 3. Viscosity
        float viscosity     = 1.0f;

        // 3.5 Surface
        //float threshold     = 0.8f;
        float threshold     = 0.1f;
        float surfCoeffi    = 0.1f;

        // 4. Gravity
        glm::vec3 gravityAcel = glm::vec3(0.0f, -10.0f, 0.0f);

        // 5. Delta Time
        float deltaTime = 0.008f;

        // 6. Damping
        float damping   = 0.01f;

        // 7. Visible
        // unsigned int neighborLevel = 4;
        // int tempNei = 4;
        float neighborLevel = 0.0005f;
        float tempNei = 4;


        float correction = 0.1f;


        float visibleCoeffi = 1.1f;
        float visibleThre   = 3.8f;


        float controlValue = 0.55f;
        float ratio = 0.4f;

    // Program ���� �Լ�
    void Get_Density_Pressure();
    void Get_Force();
    void Get_Move();
    void Find_Visible(const glm::mat4& projection, const glm::mat4& view);
    void Draw_Particles(const glm::mat4& proj, const glm::mat4& view);
    // GPU Instancing �� ���ؼ�, ���� ��ƼŬ���� �ѹ��� �׸��� �Լ�
    // Simple Program �̿�
    void Draw_GPU_Instancing(const glm::mat4& proj, const glm::mat4& view);


    // ī�޶� ���忡�� ���� ���� ���� �׸��� shadow map
    ShadowMapPtr depthFrameBuffer{};
    float cameraNear= 15.0f;
    float cameraFar = 80.0f;
    

    float offset = 0.015f;
};

#endif // __CONTEXT_H__