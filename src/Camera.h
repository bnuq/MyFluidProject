#ifndef __CAM_H__
#define __CAM_H__

#include "common.h"


CLASS_PTR(Camera)
class Camera
{
public:
    // 카메라가 바라보는 점이자 회전의 중심 피벗
    glm::vec3 CameraPivot;
        float CamToCharMaxLimit = 0.1f;
        float CamToCharMinLimit = 0.01f;

    bool CameraMove = false;
        float CameraMoveSpeed = 0.01f;

        float yawRotSpeed { 0.1f };
        float pitchRotSpeed { 0.1f };
        glm::vec2 pitchRotLimit { glm::vec2{-10.0f, 30.0f} };


    // 카메라의 위치 초기화
    glm::vec3 Position;

    // 카메라 모델 좌표계
    glm::vec3 FrontVec          = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 LeftVec           = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 UpVec             = glm::vec3(0.0f, 1.0f, 0.0f);
    // 카메라가 바라보는 방향
    glm::vec3 Direction         = glm::vec3(0.0f, 0.0f, 1.0f);
    

/* 카메라 위치, 거리 */
    float xzDist = 40.0f;
    float yDist = 30.0f;

    
    // 카메라가 따라다니는 플레이어
    CharacterPtr mainChar;



    static CameraPtr Create(CharacterPtr player);
    void Init(CharacterPtr player);
    

    // 카메라를 따라간다 + 땅바닥 높이를 바라본다
    void SetPosition();
    // 마우스를 움직이는 만큼, 카메라가 회전한다 + 바라보는 Direction 벡터도 달라진다
    void Rotate(glm::vec2 deltaPos);


private:
    Camera() {}
};

#endif  // __CAM_H__