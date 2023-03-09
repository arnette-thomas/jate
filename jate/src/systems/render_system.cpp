#include <jate/systems/render_system.h>

#include <jate/utils/utils.h>

namespace jate::systems
{
    RenderSystem::RenderSystem(rendering::ARenderer* renderer)
        : m_renderer(renderer)
    {
    }

    void RenderSystem::onComponentAdded(components::AComponent* component)
    {
        if (utils::instanceof<components::ARenderUnit>(component))
        {
            components::ARenderUnit* renderUnit = dynamic_cast<components::ARenderUnit*>(component);
            m_renderUnitComponents.insert({renderUnit->getComponentId(), renderUnit});
        }
    }
    
    void RenderSystem::onComponentRemoved(components::AComponent* component)
    {
        auto componentIt = m_renderUnitComponents.find(component->getComponentId());
        if (componentIt != m_renderUnitComponents.end())
        {
            componentIt->second->free(m_renderer);
            m_renderUnitComponents.erase(componentIt);
        }
    }

    void RenderSystem::tick()
    {
        for (const auto& renderUnit : m_renderUnitComponents)
        {
            renderUnit.second->draw(m_renderer);
        }
    }
}