#ifndef Jate_Application_H
#define Jate_Application_H

#include <jate/window/window.h>
#include <jate/rendering/renderer.h>

#include <memory>

namespace jate
{
    class Application
    {
    public:
        Application();
        ~Application();

        void run();
    
    private:
        bool m_running = false;
        Window m_window;
        
        std::unique_ptr<rendering::ARenderer> m_renderer;
    };
}

#endif