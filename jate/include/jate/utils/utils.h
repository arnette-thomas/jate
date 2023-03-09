#ifndef Jate_Utils_H
#define Jate_Utils_H

namespace jate::utils
{
    // Checks at runtime if a given ptr is an instance of a given class
    template <class Base, class T>
    inline bool instanceof(T* ptr)
    {
        return dynamic_cast<Base*>(ptr) != nullptr;
    }
}

#endif