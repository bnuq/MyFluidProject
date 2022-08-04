#ifndef __CAM_H__
#define __CAM_H__

#include "../FrameWork/common.h"


CLASS_PTR(Camera)
class Camera
{
public:
    // ī�޶� �̵�
    float CameraMoveSpeed = 0.1f;

    // ī�޶� ȸ��
    float yawRotSpeed { 0.1f };
    float pitchRotSpeed { 0.1f };
    glm::vec2 pitchRotLimit { glm::vec2{-10.0f, 30.0f} };


    // ī�޶��� ��ġ �ʱ�ȭ
    glm::vec3 Position          = glm::vec3(-4.25f, 7.7f, -19.0f);

    // ī�޶� �� ��ǥ��
    glm::vec3 FrontVec          = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 LeftVec           = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 UpVec             = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 DirVec            = glm::vec3(0.0f, 0.0f, 1.0f);

    static CameraPtr Create();


    // ���콺�� �����̴� ��ŭ, ī�޶� ȸ���Ѵ� + �ٶ󺸴� Direction ���͵� �޶�����
    void Rotate(glm::vec2 deltaPos);

    void Move(int key);

    
private:
    Camera() {}
};

#endif  // __CAM_H__