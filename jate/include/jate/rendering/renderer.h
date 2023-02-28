#ifndef Jate_Renderer_H
#define Jate_Renderer_H

#include <jate/window/window.h>

namespace jate::rendering
{
    class ARenderer
    {
    public:
        ARenderer(Window& window) : m_window(window) {}
        ~ARenderer(){}

        // Disable copy
        ARenderer(const ARenderer&) = delete;
        ARenderer& operator=(const ARenderer&) = delete;

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;

    protected:
        Window& m_window;
    };
}

#endif