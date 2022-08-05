#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <imgui.h>
#include <vector>                   // CPU 내 데이터 저장


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
    // 기본적인 함수
    static ContextUPtr Create();
    void Render();

    // 키보드, 마우스 입력 처리
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



    // Meshes => 모든 유체는 박스의 형태로만 표현
    MeshUPtr BoxMesh;
    

    // Material => 유체의 색을 표현하는 데 텍스처는 따로 쓰지 않는다
    
    // 카메라 객체
    CameraPtr MainCam;
    glm::vec3 m_cameraPos { glm::vec3(0.0f, 2.5f, 8.0f) }; // 일단 light 계산에 사용
    bool m_cameraControl { false };


    // 빛, 라이트
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
    ProgramUPtr DensityPressureCompute;     // 1. 밀도와 압력만 먼저 계산
    ProgramUPtr ForceCompute;               // 2. 입자의 알짜힘 계산
    ProgramUPtr MoveCompute;                // 3. 유체 움직이기

    ProgramUPtr MakeDepthMap;               // 4. 카메라 입장에서 렌더링 ~ depth map 을 만든다
    ProgramUPtr DrawProgram;                // 5. 화면에 실제로 유체를 그린다

    // Compute Shaders
    ShaderPtr FluidDensityPressure;          // 1. 밀도, 압력 계산 compute shader
    ShaderPtr FluidForce;                    // 2. 알짜힘을 계산하는 compute shader
    ShaderPtr FluidMove;                     // 3. 유체의 움직임을 계산하는 compute Shader




    // CPU 에서 Core Particle 데이터를 저장하는 배열 => 프레임이 바뀌어도 저장되어야 하는 정보들로 구성
    std::vector<CoreParticle> CoreParticleArray{};

    // Core Particle 들의 초기 위치 값을 넣는 함수
    void Init_CoreParticles();



    // Core Particle ~ 카메라 까지의 거리로 정렬하는 function object
    struct CoreParticle_toCamera_Compare
    {
        bool operator()(const CoreParticle& p1, const CoreParticle& p2)
        {
            // 오로지 파티클 ~ 카메라 사이의 거리를 기준으로 정렬
            return p1.toCamera > p2.toCamera;
        }
    };
    CoreParticle_toCamera_Compare cpCompare{};



    /*  
        SSBO Buffer
     */
    BufferPtr CoreParticleBuffer;                   // Core Particle 데이터를 저장, 해당 타입으로 데이터를 주고 받는다
    BufferPtr ParticleBuffer;                       // Particle 데이터, GPU 내에서만 저장하고 사용

    unsigned int CoreParticle_Index  = 1;
    unsigned int Particle_Index  = 2;



    // 각 프로그램에서 필요로 하는 Uniform Variables
    // 1. Smooth Kernel
        float SmoothKernelRadius = 1.0f;
        
    // 2. Pressure Data
        struct PressureData
        {
            float gasCoeffi   = 0.001f;
            float restDensity = 50.0f;
        };
        PressureData PD;

    // 3. Viscosity
        float viscosity       = 1.0f;

    // 4. Surface
        float surfThreshold   = 0.1f;
        float surfCoeffi      = 0.1f;

    // 5. Gravity
        glm::vec3 gravityAcel = glm::vec3(0.0f, -10.0f, 0.0f);
    
    // 6. Wave Power
        float wavePower = 3.0f;







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




    // Program 실행 함수
    void Get_Density_Pressure();                    // 1
    void Get_Force();                               // 2
    void Get_Move();                                // 3

    void Make_Depth_Map(const glm::mat4& proj, const glm::mat4& view);  // 4
    void Draw_Particles(const glm::mat4& proj, const glm::mat4& view);  // 5



    // 4 => 카메라 입장에서 깊이 값을 먼저 그리는 shadow map
    ShadowMapPtr depthFrameBuffer{};
    float cameraNear= 15.0f;
    float cameraFar = 80.0f;
    

    // 5 => 렌더링할 때 프레그먼트와 픽셀 사이 깊이 값 비교 오차
    float offset = 0.001f;
};

#endif // __CONTEXT_H__