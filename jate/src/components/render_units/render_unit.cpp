#include <jate/components/render_units/render_unit.h>

#include <jate/models/entity.h>

namespace jate::components
{
    void ARenderUnit::draw(rendering::ARenderer* renderer)
    {
        if (!m_initialized)
        {
            initialize(renderer);
        }

        rendering::PushConstantData constantData{};
        constantData.transform = m_entity->getTransform().getMatrix();
        renderer->drawIndexed(m_allocatedData.verticesSlot, m_allocatedData.indicesSlot, constantData);
    }

    void ARenderUnit::initialize(rendering::ARenderer* renderer)
    {
        m_allocatedData = allocateRenderingData(renderer);
        m_initialized = true;
    }

    void ARenderUnit::free(rendering::ARenderer* renderer)
    {
        renderer->freeVertexData(m_allocatedData.verticesSlot);
        renderer->freeIndexData(m_allocatedData.indicesSlot);
        m_initialized = false;
    }
}