#include <jate/models/entity.h>

#include <jate/models/world.h>

namespace jate::models
{
    Entity::Entity(World* world) : m_world(world)
    {
        static uint32_t s_nextEntityId = 0;
        m_id = s_nextEntityId;
        s_nextEntityId++;
    }

    Entity::~Entity()
    {
        for(components::AComponent& comp : m_components)
        {
            m_world->onComponentRemoved(comp);
        }
    }

    void Entity::signalComponentAddedToWorld(components::AComponent &component)
    {
        m_world->onComponentAdded(component);
    }
}