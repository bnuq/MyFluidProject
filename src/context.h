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
    glm::vec4 m_clearColor { glm::vec4(0.1f, 0.2f, 0.3f, 0.0f) };

    // Mouse
    glm::vec2 m_prevMousePos { glm::vec2(0.0f) };

    // Programs => ��ü�� �׷����� ���α׷�, ��ü�� ����ϴ� ���α׷�
    ProgramUPtr FluidProgram;
    ProgramUPtr ComputeProgram;


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


    
    // ��ü�� �������� ����ϴ� Compute Shader
    ShaderPtr FluidComputeShader;


    // ��ü�� �����ϴ� Particle Struct
    // Particle �����͸� �����ϴ� �迭 => CPU
    std::vector<Particle> ParticleArray{};


    // Particle ���� �ʱ� ��ġ ���� �ִ� �Լ�
    void InitParticles();

    // Particle ���� ī�޶� ������ �Ÿ��� �����ϴ� function object
    struct ParticleCompare
    {
        bool operator()(const Particle& p1, const Particle& p2)
        {
            // ī�޶������ �Ÿ��� �� �� => ª�� ��, ������ �����Ѵ�
            return p1.ToCamera > p2.ToCamera;
        }
    };
    



    /*  SSBO Buffer

        Particle �����͸� �����ϴ� SSBO ���� => GPU ���� ������ ����
        ���� ���۸� 2�� �����Ѵ�
        ���ʿ��� �а�, �ٸ� ������ �����
        ���� �� ���۸� ����, Compute Program �� �Է°� ����� �����Ƽ� ����ǵ��� �Ѵ�
     */
    BufferPtr InputBuffer;
    BufferPtr OutputBuffer;

    unsigned int Input_Index  = 1;
    unsigned int Output_Index = 2;

};

#endif // __CONTEXT_H__