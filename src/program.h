#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "common.h"
#include "shader.h"

/* 
    Program ?´?˜?Š¤
        GL program object ë¥? ê´?ë¦?
    
    GL program
        ?—¬?Ÿ¬ ê°œì˜ ì»´íŒŒ?¼ ?œ shader ?“¤?„ ?•©ì³ë†“??? ê²?
        programmable, ?‚´ê°? ?ˆ˜? •?•  ?ˆ˜ ?ˆ?Š” ?…°?´?”?“¤?„ ëª¨ì•„?†“?•˜?‹¤
        ?–´?–¤ ?…°?´?”ë¥? ?‚¬?š©?•  ì§? ê²°ì •?•  ?ˆ˜ ?ˆ?‹¤
        ?…°?´?”?“¤?— ?“¤?–´ê°??Š” Uniform ê´?ë¦?
 */
CLASS_PTR(Program)
class Program
{
public:
    static ProgramUPtr Create(const std::vector<ShaderPtr>& shaders);
    static ProgramUPtr Create(const std::string& vertShaderFilename, const std::string& fragShaderFilename);
    ~Program();
    /* 
        ê´?ë¦¬í•˜ê³? ?ˆ?Š” GL program ?„ ?‚¬?š©?•˜ê³? ?‹¶?„ ?•Œ ?‚¬?š©
            GL program ?´?¼?Š” ê²ƒì??, ì»´íŒŒ?¼ ?œ ?…°?´?”?“¤?„ ëª¨ì•„?†“??? ê²?
            ì¦?, ?‚´ê°? ?‘?„±?•œ ?–´?–¤ ?…°?´?”?“¤?„ ?‚¬?š©?•˜ê³? ?‹¶?„ ?•Œ ?˜¸ì¶?
     */    
    void Use() const;

    // Return GL program handle id
    uint32_t Get() const { return m_program; }
    /* 
        shader ?— uniform ë³??ˆ˜ë¥? ?„˜ê¸°ëŠ” ?•¨?ˆ˜?“¤
        ?„˜ê¸°ëŠ” Uniform ë³??ˆ˜?— ë§ê²Œ function overloading

        Uniform ë³??ˆ˜?˜ ?´ë¦? ë¬¸ì?—´?„ ?†µ?•´?„œ ? ‘ê·¼í•œ?‹¤
     */
    void SetUniform(const std::string& name, unsigned int value) const;
    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, bool value) const;
    void SetUniform(const std::string& name, float value) const;

    void SetUniform(const std::string& name, const glm::vec2& value) const;
    void SetUniform(const std::string& name, const glm::vec3& value) const;
    void SetUniform(const std::string& name, const glm::vec4& value) const;

    void SetUniform(const std::string& name, const glm::mat4& value) const;

private:
    Program() {}
    // compile ?•œ GL shader ?“¤?„ Link => ?˜¨? „?•œ GL program ?„ ë§Œë“ ?‹¤
    bool Link(const std::vector<ShaderPtr>& shaders);

    // Program Object id
    uint32_t m_program { 0 };
};

#endif // __PROGRAM_H__