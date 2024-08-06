#include "DebugWindow.h"

class DebugWindowGLFW : public DebugWindow
{
public:
    DebugWindowGLFW();
    ~DebugWindowGLFW();

protected:
    virtual void init() override;
    virtual void cleanup() override;
    virtual void drawImpl() override;
};