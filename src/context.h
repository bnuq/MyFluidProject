#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <imgui.h>
#include <vector>
#include <map>
#include <deque>


#include "common.h"
#include "program.h"
#include "mesh.h"
#include "framebuffer.h"


#include "Camera.h"
#include "Particle.h"


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
    glm::vec4 m_clearColor { glm::vec4(0.1f, 0.2f, 0.3f, 0.0f) };

    // Mouse
    glm::vec2 m_prevMousePos { glm::vec2(0.0f) };


    // Programs
    ProgramUPtr DensityPressureCompute;     // 밀도와 압력만 먼저 계산
    ProgramUPtr ForceCompute;               // 입자의 알짜힘 계산
    ProgramUPtr MoveCompute;                // 유체 움직이기
    ProgramUPtr DrawProgram;                // 유체 그리기


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





    // Compute Shaders
    ShaderPtr FluidDensityPressure;          // 밀도, 압력 계산 compute shader
    ShaderPtr FluidForce;                    // 알짜힘을 계산하는 compute shader
    ShaderPtr FluidMove;                     // 유체의 움직임을 계산하는 Compute Shader




    // CPU 에서 Core Particle 데이터를 저장하는 배열 => 프레임이 바뀌어도 저장되어야 하는 정보들로 구성
    std::vector<CoreParticle> CoreParticleArray{};


        // 일단 중간중간에 Particle 내용을 확인하기 위해 사용하는 array
        std::vector<Particle> ParticleArray{};


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


    // 파티클들을 렌더링하기 전에, visible 데이터를 이용해서 정렬하는 function object
    struct CoreParticle_visible_Compare
    {
        bool operator()(const CoreParticle& p1, const CoreParticle& p2)
        {
            // 둘다 보이는 경우
            if(p1.visible != 0 && p2.visible != 0)
                // 카메라 거리를 이용, 거리가 더 먼 것이 앞으로 온다
                return p1.toCamera > p2.toCamera;
            else
                // 그 이외에는 보이는 것이 앞으로 오게 정렬한다
                return p1.visible > p2.visible;
        }
    };
    

    // visible core particles 의 갯수
    unsigned int visibleCount = 0;


    /*  
        SSBO Buffer
     */
    BufferPtr CoreParticleBuffer;       // Core Particle 데이터를 저장, 해당 타입으로 데이터를 주고 받는다
    BufferPtr ParticleBuffer;           // Particle 데이터, GPU 내에서만 저장하고 사용
    BufferPtr CountBuffer;              // visible count 값을 읽어오는 버퍼


    unsigned int CoreParticle_Index  = 1;
    unsigned int Particle_Index  = 2;
    unsigned int Count_Index = 5;



    // 각 프로그램에서 필요로 하는 Uniform Variables
        // 1. Smooth Kernel
        float SmoothKernelRadius = 1.6f;
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
        float threshold     = 0.3f;
        float surfCoeffi    = 0.1f;

        // 4. Gravity
        glm::vec3 gravityAcel = glm::vec3(0.0f, -10.0f, 0.0f);

        // 5. Delta Time
        float deltaTime = 0.008f;

        // 6. Damping
        float damping   = 0.2f;

        // 7. Visible
        // unsigned int neighborLevel = 4;
        // int tempNei = 4;
        float neighborLevel = 0.0005f;
        float tempNei = 4;


    // Program 실행 함수
    void Get_Density_Pressure();
    void Get_Force();
    void Get_Move();
    void Draw_Particles(const glm::mat4& proj, const glm::mat4& view);



    
};

#endif // __CONTEXT_H__