#ifndef Jate_RenderSystem_H
#define Jate_RenderSystem_H

#include <jate/systems/system.h>
#include <jate/rendering/renderer.h>
#include <jate/components/render_units/render_unit.h>

#include <unordered_map>

namespace jate::systems
{
    class RenderSystem : public ASystem
    {
    public:
        RenderSystem(rendering::ARenderer* renderer);

        virtual void onComponentAdded(components::AComponent& component) override;
        virtual void onComponentRemoved(components::AComponent& component) override;
        virtual void tick() override;
    
    private:
        rendering::ARenderer* m_renderer;

        std::unordered_map<uint32_t, std::reference_wrapper<components::ARenderUnit>> m_renderUnitComponents;
    };
}

#endif