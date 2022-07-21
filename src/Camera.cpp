#include "Camera.h"

CameraPtr Camera::Create()
{
    auto camera = CameraPtr(new Camera());
    return camera;
}

// ī�޶� ȸ��, ���콺 �Է¿� ���� ī�޶� ��ǥ��� direction ���Ͱ� ȸ���Ѵ�
void Camera::Rotate(glm::vec2 deltaPos)
{
    auto yawAngle = (-deltaPos.x) * yawRotSpeed;
    if(yawAngle < 0)    yawAngle += 360;
    if(yawAngle > 360)  yawAngle -= 360;

    auto yawRotate = glm::rotate(glm::mat4(1.0f), glm::radians(yawAngle), UpVec);

    FrontVec    = glm::vec3(yawRotate * glm::vec4(FrontVec, 0.0f));
    LeftVec     = glm::vec3(yawRotate * glm::vec4(LeftVec, 0.0f));
    DirVec      = FrontVec;

    // pitchAngle �� ������ �ִ� + ���� �����Ѵ�
    auto pitchAngle = (deltaPos.y) * pitchRotSpeed;
        pitchAngle = glm::clamp(pitchAngle, pitchRotLimit.x, pitchRotLimit.y);

    // Direction ����
    auto pitchRotate = glm::rotate(glm::mat4(1.0f), glm::radians(pitchAngle), LeftVec); // �ٲ� LeftVec �� ���ؼ�
    FrontVec = glm::vec3(pitchRotate * glm::vec4(FrontVec, 0.0f));
    DirVec = FrontVec;
}

// �Է¹��� Ű�� ��������, ī�޶��� ��ġ�� �̵�, ��ǥ��� ������ �ʴ´�
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