#ifndef Jate_Concepts_H
#define Jate_Concepts_H

#include <concepts>
#include <jate/components/component.h>

namespace jate::utils::concepts
{
    template <typename T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;

    template <typename T>
    concept component_type = std::is_base_of<components::AComponent, T>::value;
}

#endif