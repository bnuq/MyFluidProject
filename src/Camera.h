#ifndef __CAM_H__
#define __CAM_H__

#include "../FrameWork/common.h"


CLASS_PTR(Camera)
class Camera
{
public:
    // 카메라 이동
    float CameraMoveSpeed = 0.1f;

    // 카메라 회전
    float yawRotSpeed { 0.1f };
    float pitchRotSpeed { 0.1f };
    glm::vec2 pitchRotLimit { glm::vec2{-10.0f, 30.0f} };


    // 카메라의 위치 초기화
    glm::vec3 Position          = glm::vec3(-4.25f, 7.7f, -19.0f);

    // 카메라 모델 좌표계
    glm::vec3 FrontVec          = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 LeftVec           = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 UpVec             = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 DirVec            = glm::vec3(0.0f, 0.0f, 1.0f);

    static CameraPtr Create();


    // 마우스를 움직이는 만큼, 카메라가 회전한다 + 바라보는 Direction 벡터도 달라진다
    void Rotate(glm::vec2 deltaPos);

    void Move(int key);

    
private:
    Camera() {}
};

#endif  // __CAM_H__