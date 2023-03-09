#ifndef Jate_Renderer_H
#define Jate_Renderer_H

#include <jate/window/window.h>

#include <jate/rendering/data_structs.h>

#include <jate/models/transform.h>

namespace jate::rendering
{
    using renderer_memory_slot_id = uint32_t;

    class ARenderer
    {
    public:
        virtual ~ARenderer(){}

        // Disable copy
        ARenderer(const ARenderer&) = delete;
        ARenderer& operator=(const ARenderer&) = delete;

        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;

        /// @brief Allocates memory to store vertex data. This MUST be freed using the corresponding free() method.
        /// @param vertices An array of vertex data to be stored in renderer memory
        /// @return The memory slot id of the allocated data
        virtual renderer_memory_slot_id allocateVertexData(const std::vector<VertexData>& vertices) = 0;

        /// @brief Frees vertex data at the given slotId
        virtual void freeVertexData(renderer_memory_slot_id slotId) = 0;

        /// @brief Allocates memory to store index data. This MUST be freed using the corresponding free() method.
        /// @param indices An array of index data to be stored in renderer memory
        /// @return the memory slot id of the allocated data
        virtual renderer_memory_slot_id allocateIndexData(const std::vector<uint32_t>& indices) = 0;

        /// @brief Frees index data at the given slotId
        virtual void freeIndexData(renderer_memory_slot_id slotId) = 0;

        virtual void drawIndexed(renderer_memory_slot_id verticesSlotId, renderer_memory_slot_id indicesSlotId, const PushConstantData& pushConstantData) = 0;

    protected:
        ARenderer(Window& window) : m_window(window) {}
        
        Window& m_window;
    };
}

#endif