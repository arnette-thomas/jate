#ifndef Jate_RenderUnit_H
#define Jate_RenderUnit_H

#include <jate/components/component.h>
#include <jate/rendering/renderer.h>

namespace jate::components
{
    class ARenderUnit : public AComponent
    {
    public:
        ARenderUnit(jate::models::Entity* entity) : AComponent(entity) {}

        void draw(rendering::ARenderer* renderer);
        void free(rendering::ARenderer* renderer);

    private:
        void initialize(rendering::ARenderer* renderer);

    protected:

        struct AllocatedRenderingData
        {
            rendering::renderer_memory_slot_id verticesSlot, indicesSlot;
        };

        /// @brief This method initializes renderer slots with the right vertex / index data
        ///        It will be used by the draw() method.
        virtual AllocatedRenderingData allocateRenderingData(rendering::ARenderer* renderer) const = 0;

        bool m_initialized = false;
        AllocatedRenderingData m_allocatedData;
    };
}

#endif