#ifndef Jate_RenderUnit_H
#define Jate_RenderUnit_H

#include <jate/components/component.h>
#include <jate/rendering/data_structs.h>

namespace jate::components
{
    class ARenderUnit : public AComponent
    {
    public:
        ARenderUnit(jate::models::Entity& entity) : AComponent(entity) {}
    };
}

#endif