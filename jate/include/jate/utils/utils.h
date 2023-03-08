#ifndef Jate_Utils_H
#define Jate_Utils_H

#include <type_traits>

namespace jate::utils
{
    // Checks at runtime if a given ptr is an instance of a given class
    template <class Base, class T>
    inline bool instanceof(const T*)
    {
        return std::is_base_of<Base, T>::value;
    }
    template <class Base, class T>
    inline bool instanceof(const T&)
    {
        return std::is_base_of<Base, T>::value;
    }
}

#endif