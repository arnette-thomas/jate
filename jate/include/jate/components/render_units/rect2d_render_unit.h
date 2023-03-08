#ifndef Jate_Rect2DRenderUnit_H
#define Jate_Rect2DRenderUnit_H

#include <jate/components/render_units/render_unit.h>

namespace jate::components
{
    class Rect2DRenderUnit : public ARenderUnit
    {
    public:
        Rect2DRenderUnit(jate::models::Entity& entity) : ARenderUnit(entity) {}
    };
}

#endif