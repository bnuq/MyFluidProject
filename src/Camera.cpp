#include "Camera.h"

CameraPtr Camera::Create()
{
    auto camera = CameraPtr(new Camera());
    return camera;
}

// 카메라 회전, 마우스 입력에 따라서 카메라 좌표계와 direction 벡터가 회전한다
void Camera::Rotate(glm::vec2 deltaPos)
{
    auto yawAngle = (-deltaPos.x) * yawRotSpeed;
    if(yawAngle < 0)    yawAngle += 360;
    if(yawAngle > 360)  yawAngle -= 360;

    auto yawRotate = glm::rotate(glm::mat4(1.0f), glm::radians(yawAngle), UpVec);

    FrontVec    = glm::vec3(yawRotate * glm::vec4(FrontVec, 0.0f));
    LeftVec     = glm::vec3(yawRotate * glm::vec4(LeftVec, 0.0f));
    DirVec      = FrontVec;

    // pitchAngle 은 제한이 있다 + 값을 누적한다
    auto pitchAngle = (deltaPos.y) * pitchRotSpeed;
        pitchAngle = glm::clamp(pitchAngle, pitchRotLimit.x, pitchRotLimit.y);

    // Direction 세팅
    auto pitchRotate = glm::rotate(glm::mat4(1.0f), glm::radians(pitchAngle), LeftVec); // 바뀐 LeftVec 에 대해서
    FrontVec = glm::vec3(pitchRotate * glm::vec4(FrontVec, 0.0f));
    DirVec = FrontVec;
}

// 입력받은 키의 방향으로, 카메라의 위치가 이동, 좌표계는 변하지 않는다
void Camera::Move(int key)
{
    switch (key)
    {
        case GLFW_KEY_W:
            Position += FrontVec * CameraMoveSpeed;
            break;
        
        case GLFW_KEY_S:
            Position -= FrontVec * CameraMoveSpeed;
            break;

        case GLFW_KEY_A:
            Position += LeftVec * CameraMoveSpeed;
            break;

        case GLFW_KEY_D:
            Position -= LeftVec * CameraMoveSpeed;
            break;

        case GLFW_KEY_Q:
            Position += UpVec * CameraMoveSpeed;
            break;

        case GLFW_KEY_E:
            Position -= UpVec * CameraMoveSpeed;
            break;

        default:
            break;
    }
}