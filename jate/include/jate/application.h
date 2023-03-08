#ifndef Jate_Application_H
#define Jate_Application_H

#include <jate/window/window.h>
#include <jate/rendering/renderer.h>
#include <jate/models/world.h>
#include <jate/systems/system.h>

#include <memory>
#include <map>

namespace jate
{
    class Application
    {
    public:
        Application();
        ~Application();

        models::World* createWorld();

        void run();
    
    private:
        bool m_running = false;
        Window m_window;
        
        std::unique_ptr<rendering::ARenderer> m_renderer;
        std::unique_ptr<models::World> m_world;
        
    };
}

#endif