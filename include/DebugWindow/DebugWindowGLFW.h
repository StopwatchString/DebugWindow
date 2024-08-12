#include "DebugWindow.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

static void glfw_error_callback(int error, const char* description);

class DebugWindowGLFW : public DebugWindow
{
public:
    DebugWindowGLFW();
    ~DebugWindowGLFW();

protected:
    virtual void init() override;
    virtual void cleanup() override;
    virtual void drawImpl() override;
    virtual void toggleVsync() override;
    virtual void pushOpenGLState() override;
    virtual void popOpenGLState() override;

private:
    GLFWwindow* m_Window{ nullptr };
    GLFWwindow* m_ReturnOpenGLContext{ nullptr };
};