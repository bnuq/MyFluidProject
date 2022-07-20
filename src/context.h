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

    // Programs => 유체를 그려내는 프로그램, 유체를 계산하는 프로그램
    ProgramUPtr FluidProgram;
    ProgramUPtr ComputeProgram;


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


    
    // 유체의 움직임을 계산하는 Compute Shader
    ShaderPtr FluidComputeShader;


    // 유체를 구성하는 Particle Struct
    // Particle 데이터를 저장하는 배열 => CPU
    std::vector<Particle> ParticleArray{};


    // Particle 들의 초기 위치 값을 넣는 함수
    void InitParticles();

    // Particle 들을 카메라 까지의 거리로 정렬하는 function object
    struct ParticleCompare
    {
        bool operator()(const Particle& p1, const Particle& p2)
        {
            // 카메라까지의 거리가 긴 것 => 짧은 것, 순서로 정렬한다
            return p1.ToCamera > p2.ToCamera;
        }
    };
    



    /*  SSBO Buffer

        Particle 데이터를 저장하는 SSBO 버퍼 => GPU 에서 정보를 저장
        같은 버퍼를 2개 생성한다
        한쪽에서 읽고, 다른 쪽으로 출력해
        이후 두 버퍼를 스왑, Compute Program 에 입력과 출력이 번갈아서 연결되도록 한다
     */
    BufferPtr InputBuffer;
    BufferPtr OutputBuffer;

    unsigned int Input_Index  = 1;
    unsigned int Output_Index = 2;

};

#endif // __CONTEXT_H__