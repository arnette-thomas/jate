#ifndef Jate_System_H
#define Jate_System_H

#include <jate/components/component.h>

namespace jate::systems
{
    enum SystemEnum
    {
        RENDER_SYSTEM
    };

    class ASystem
    {
    public:
        virtual void onComponentAdded(components::AComponent* component) = 0;
        virtual void onComponentRemoved(components::AComponent* component) = 0;
        virtual void tick() = 0;
    };
}

#endif