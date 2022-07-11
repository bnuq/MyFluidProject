#[[
    새롭게 만든 Module

    기존 CMakeLists 파일이 너무 길어지므로 외부 라이브러리 설정에 필요한 내용만 따로 빼서 정리한다
    Dependency 관련 설정을 저장하는 파일

    CMake 를 이용한 외부 라이브러리 사용을 하고 싶다 => 추가, 설정
]]#



#[[
    include => 추가적으로 사용하려는 기능 추가
    ExternalProject 라는 Standard Module 을 사용
        외부 라이브러리를 추가하는 기능

        ExternalProject_Add function
            깃 저장소를 통해서 외부 패키지를 다운받을 수 있다
]]#
include(ExternalProject)



#[[
    ${PROJECT_BINARY_DIR}   = 실행파일이 만들어지는 디렉토리   => 여기서는 build dir 내부
    ${DEP_INSTALL_DIR}      = build/install                 => 외부 패키지를 저장하는 디렉토리
    ${DEP_INCLUDE_DIR}      = build/install/include         => 헤더 파일을 저장
    ${DEP_LIB_DIR}          = build/install/lib             => 라이브러리를 저장
]]#
set(DEP_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)  
set(DEP_INCLUDE_DIR ${DEP_INSTALL_DIR}/include)
set(DEP_LIB_DIR ${DEP_INSTALL_DIR}/lib)



# spdlog: fast logger library 추가
ExternalProject_Add(
    dep_spdlog                              # 프로젝트를 컴파일할 때 대표 이름
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"   # 외부 라이브러리 주소
    GIT_TAG "v1.x"                          # 사용하려는 버전, 깃 저장소의 브렌치, 태그 중 하나를 가져옴
    GIT_SHALLOW 1                           # On/Off, shallow = 커밋 중 가장 최신만 가져옴 => 을 활성화


    # git clone 이후 update, patch step 에서 아무 작업도 하지 않는다는 걸 명시
    UPDATE_COMMAND ""                       # git version 을 최신으로 업데이트
    PATCH_COMMAND ""                        # 코드 내에서 수정사항이 있는 경우 수정


    # CMake Configure 를 하는 데 필요한 arguments 값 대입
    # install dir 에 패키지를 install 해라
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}


    TEST_COMMAND ""                         # test step, 자동 테스트가 이루어지는 단계
)


# Dependency 리스트에 해당 패키지 이름을 추가
set(DEP_LIST ${DEP_LIST} dep_spdlog)

# Dependency 라이브러리에 해당 라이브러리 추가
# CMake 의 Generator 문법, Configure 의 모드 설정 => Debug Mode
set(DEP_LIBS ${DEP_LIBS} spdlog$<$<CONFIG:Debug>:d>)



# glfw 라이브러리 추가 => Rendering 외 작업
ExternalProject_Add(
    dep_glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.3.7"
    GIT_SHALLOW 1
    UPDATE_COMMAND "" PATCH_COMMAND "" TEST_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}

        # GLFW 파일 외에, 예제 파일, 테스트 파일, 문서 파일이 존재
        # 전부 다 필요 없으니까, 제외시킨다는 옵션
        # git repo => CMakeLists 에 option 으로 지정되어 있다
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_TESTS=OFF
        -DGLFW_BUILD_DOCS=OFF
    )
set(DEP_LIST ${DEP_LIST} dep_glfw)
set(DEP_LIBS ${DEP_LIBS} glfw3)



# glad 라이브러리 추가 => opengl 함수를, 운영체제에 맞게 가져올 수 있다
ExternalProject_Add(
    dep_glad
    GIT_REPOSITORY "https://github.com/Dav1dde/glad"
    GIT_TAG "v0.1.34"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DGLAD_INSTALL=ON
    TEST_COMMAND ""
    )
set(DEP_LIST ${DEP_LIST} dep_glad)
set(DEP_LIBS ${DEP_LIBS} glad)



# stb 라이브러리 => image loading 을 도와줌
ExternalProject_Add(
    dep_stb
    GIT_REPOSITORY "https://github.com/nothings/stb"
    GIT_TAG "master"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    # 이 라이브러리는 빌드할 필요가 없다, 헤더 파일만 사용하니까
    # 그래서 configure, build 명령어가 없다
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""

    TEST_COMMAND ""

    # 헤더 파일 복사 => 여러 헤더 파일 중 stb_image.h 파일만 사용
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
        # build 폴더 내 저장된 것 중 stb_image.h 파일을
        ${PROJECT_BINARY_DIR}/dep_stb-prefix/src/dep_stb/stb_image.h
        # include 폴더 내에 저장
        ${DEP_INSTALL_DIR}/include/stb/stb_image.h
    )
set(DEP_LIST ${DEP_LIST} dep_stb)



# glm 라이브러리 추가
ExternalProject_Add(
    dep_glm
    GIT_REPOSITORY "https://github.com/g-truc/glm"
    GIT_TAG "0.9.9.8"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_BINARY_DIR}/dep_glm-prefix/src/dep_glm/glm
    ${DEP_INSTALL_DIR}/include/glm
    )
set(DEP_LIST ${DEP_LIST} dep_glm)



# 직접 다운 받은 imgui 파일을 이용해서, 라이브러리를 직접 생성
add_library( # 소스코드들 => 라이브러리를 만듦
    imgui
    # 컴파일할 소스코드들을 추가
    # 컴파일은 원래 소스코드로만 하는거다, 헤더파일은 추가 변경할 때 필요
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
    )
target_include_directories(imgui PRIVATE ${DEP_INCLUDE_DIR})
add_dependencies(imgui ${DEP_LIST})
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(DEP_LIST ${DEP_LIST} imgui)
set(DEP_LIBS ${DEP_LIBS} imgui)



# assimp
ExternalProject_Add(
    dep_assimp
    GIT_REPOSITORY "https://github.com/assimp/assimp"
    GIT_TAG "v5.0.1"
    GIT_SHALLOW 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
        -DBUILD_SHARED_LIBS=OFF
        -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
        -DASSIMP_BUILD_TESTS=OFF
        -DASSIMP_INJECT_DEBUG_POSTFIX=OFF
        -DASSIMP_BUILD_ZLIB=ON
    TEST_COMMAND ""
    )
set(DEP_LIST ${DEP_LIST} dep_assimp)
set(DEP_LIBS ${DEP_LIBS}
    assimp-vc143-mt$<$<CONFIG:Debug>:d>
    zlibstatic$<$<CONFIG:Debug>:d>
    IrrXML$<$<CONFIG:Debug>:d>
)