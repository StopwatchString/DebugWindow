/*
 Class:          DebugWindowGLFW
 Author:         Mason Speck

  Implementation of DebugWindow class but using GLFW
  as the platform backend for initializing a window and
  creating an OpenGL context.

  This is pretty clean to implement with Imgui and functions
  well, but has the drawback of requiring extra headers and
  linking to the glfw3.lib (or whatever other way you want to
  get their code into your application). Also, if you drop this
  into a program that already has GLFW, or even worse, an older
  version of GLFW, then the dream of a 'drop-in hassle-free debug window'
  goes out the window. Use this when you can, otherwise use the Win32 impl.
 */

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
    virtual void implementationDrawWrapper() override;
    virtual void toggleVsync() override;
    virtual void pushOpenGLContext() override;
    virtual void popOpenGLContext() override;

private:
    GLFWwindow* m_Window{ nullptr };
    GLFWwindow* m_ReturnOpenGLContext{ nullptr };
};