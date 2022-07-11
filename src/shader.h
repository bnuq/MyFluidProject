#ifndef _SHADER_H_
#define _SHADER_H_

#include "common.h"

CLASS_PTR(Shader)
class Shader
{ 
public:
    static ShaderUPtr CreateFromFile(const std::string& filename, GLenum shaderType);
    ~Shader();

    uint32_t Get() const
    {
        
        return m_shader;
    }   // GL shader obj handle id

private:
    Shader() {}
    bool LoadFile(const std::string& filename, GLenum shaderType);

    uint32_t m_shader { 0 }; // OpenGL shader handle id
};

#endif // _SHADER_H_