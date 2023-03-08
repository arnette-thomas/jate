#ifndef Jate_World_H
#define Jate_World_H

#include <jate/rendering/renderer.h>
#include <jate/systems/system.h>
#include <jate/models/entity.h>

#include <vector>
#include <map>
#include <memory>

namespace jate { class Application; }

namespace jate::models
{
    class World
    {
    public:
        World(Application& app, rendering::ARenderer* renderer);

        Entity& spawnEntity();

        inline Application& getApplication() const { return m_application; }
        void tickSystems();

        void onComponentAdded(components::AComponent& component);
        void onComponentRemoved(components::AComponent& component);

    private:
        void init_registerSystems();

        std::vector<Entity> m_entities;
        Application& m_application;
        std::map<systems::SystemEnum, std::unique_ptr<systems::ASystem>> m_systems;

        rendering::ARenderer* m_renderer;

    };
}

#endif