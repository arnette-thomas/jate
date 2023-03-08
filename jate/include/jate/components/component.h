#ifndef Jate_Component_H
#define Jate_Component_H

#include <stdint.h>

namespace jate::models { class Entity; }

namespace jate::components
{
    class AComponent
    {
    public:
        inline uint32_t getComponentId() const { return m_id; }
        virtual ~AComponent(){}

    protected:
        // EVERY derived class should define a constructor with the same arguments as this one.
        AComponent(jate::models::Entity& entity) : m_entity(entity)
        {
            static uint32_t s_nextComponentId = 0;
            m_id = s_nextComponentId;
            s_nextComponentId++;
        }

        uint32_t m_id;
        jate::models::Entity& m_entity;
    };
}

#endif