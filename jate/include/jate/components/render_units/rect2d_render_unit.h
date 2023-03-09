#ifndef Jate_Rect2DRenderUnit_H
#define Jate_Rect2DRenderUnit_H

#include <jate/components/render_units/render_unit.h>

namespace jate::components
{
    class Rect2DRenderUnit : public ARenderUnit
    {
    public:
        Rect2DRenderUnit(jate::models::Entity* entity) : ARenderUnit(entity) {}

        void setRect(float centerX, float centerY, float width, float height);

    protected:
        virtual AllocatedRenderingData allocateRenderingData(rendering::ARenderer* renderer) const override;

    private:
        float m_centerX = 0.f, m_centerY = 0.f;
        float m_width = 1.f, m_height = 1.f;
    };
}

#endif