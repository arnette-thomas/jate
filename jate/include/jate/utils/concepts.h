#ifndef Jate_Concepts_H
#define Jate_Concepts_H

#include <concepts>

namespace jate::utils::concepts
{
    template <typename T>
    concept arithmetic = std::integral<T> || std::floating_point<T>;
}

#endif