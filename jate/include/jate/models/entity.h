#ifndef Jate_Entity_H
#define Jate_Entity_H

#include <jate/components/component.h>
#include <jate/models/transform.h>
#include <jate/utils/concepts.h>

#include <vector>
#include <memory>

namespace jate::models
{
    class World;

    class Entity
    {
    public:
        Entity(World* world);
        ~Entity();

        Entity(const Entity&) = delete;

        inline Transform& getTransform() { return m_transform; }

        template<utils::concepts::component_type Comp>
        Comp* addComponent()
        {
            Comp* component = dynamic_cast<Comp*>(m_components.emplace_back(std::make_unique<Comp>(this)).get());
            signalComponentAddedToWorld(component);
            return component;
        }

    private:
        // Triggers the onComponentAdded in World.
        // This must be defined in cpp, since it requires the full declaration of the World class.
        void signalComponentAddedToWorld(components::AComponent* component);

        World* m_world;
        uint32_t m_id;
        Transform m_transform;
        std::vector<std::unique_ptr<components::AComponent>> m_components;
    };
}

#endif