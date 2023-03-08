#include <jate/models/world.h>

#include <jate/systems/render_system.h>

namespace jate::models
{
    World::World(Application& app, rendering::ARenderer* renderer) : m_application(app), m_renderer(renderer)
    {
        init_registerSystems();
    }

    void World::init_registerSystems()
    {
        m_systems.emplace(systems::SystemEnum::RENDER_SYSTEM, std::make_unique<systems::RenderSystem>(m_renderer));
    }

    Entity& World::spawnEntity()
    {
        Entity& spawnedEntity = m_entities.emplace_back(this);
        return spawnedEntity;
    }

    void jate::models::World::tickSystems()
    {
        for (const auto& system : m_systems)
        {
            system.second->tick();
        }
    }

    void World::onComponentAdded(components::AComponent& component)
    {
        for (const auto& system : m_systems)
        {
            system.second->onComponentAdded(component);
        }
    }

    void World::onComponentRemoved(components::AComponent& component)
    {
        for (const auto& system : m_systems)
        {
            system.second->onComponentRemoved(component);
        }
    }
}