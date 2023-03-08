#include <jate/systems/render_system.h>

#include <jate/utils/utils.h>

namespace jate::systems
{
    RenderSystem::RenderSystem(rendering::ARenderer* renderer)
        : m_renderer(renderer)
    {
    }

    void RenderSystem::onComponentAdded(components::AComponent& component)
    {
        if (utils::instanceof<components::ARenderUnit>(component))
        {
            components::ARenderUnit& renderUnit = dynamic_cast<components::ARenderUnit&>(component);
            m_renderUnitComponents.insert({renderUnit.getComponentId(), std::ref(renderUnit)});
        }
    }
    
    void RenderSystem::onComponentRemoved(components::AComponent& component)
    {
        m_renderUnitComponents.erase(component.getComponentId());
    }

    void RenderSystem::tick()
    {

    }
}