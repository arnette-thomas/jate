#ifndef Jate_Vectors_H
#define Jate_Vectors_H

#include <jate/utils/concepts.h>

namespace jate::maths
{
    // --- VECTOR 2D ---

    // Generic struct definition
    template<jate::utils::concepts::arithmetic T>
    struct Vector2
    {
        T x, y;

        Vector2(T xCoord, T yCoord) : x(xCoord), y(yCoord) {}

        // simple vectors
        static Vector2<T> zero;
        static Vector2<T> one;
        static Vector2<T> up;
        static Vector2<T> left;
    };

    // Static members initialization

    template<jate::utils::concepts::arithmetic T>
    Vector2<T> Vector2<T>::zero = Vector2<T>(static_cast<T>(0), static_cast<T>(0));

    template<jate::utils::concepts::arithmetic T>
    Vector2<T> Vector2<T>::one = Vector2<T>(static_cast<T>(1), static_cast<T>(1));

    template<jate::utils::concepts::arithmetic T>
    Vector2<T> Vector2<T>::up = Vector2<T>(static_cast<T>(0), static_cast<T>(1));

    template<jate::utils::concepts::arithmetic T>
    Vector2<T> Vector2<T>::left = Vector2<T>(static_cast<T>(1), static_cast<T>(0));

    // Aliases
    using Vector2i = Vector2<int32_t>;
    using Vector2ui = Vector2<uint32_t>;
    using Vector2f = Vector2<float>;
    using Vector2d = Vector2<double>;


    // --- VECTOR 3D

    // Generic struct definition
    template<jate::utils::concepts::arithmetic T>
    struct Vector3
    {
        T x, y, z;

        Vector3(T xCoord, T yCoord, T zCoord) : x(xCoord), y(yCoord), z(zCoord) {}

        // simple vectors
        static Vector3<T> zero;
        static Vector3<T> one;
        static Vector3<T> up;
        static Vector3<T> left;
        static Vector3<T> back;
    };

    // Static members initialization

    template<jate::utils::concepts::arithmetic T>
    Vector3<T> Vector3<T>::zero = Vector3<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));

    template<jate::utils::concepts::arithmetic T>
    Vector3<T> Vector3<T>::one = Vector3<T>(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1));

    template<jate::utils::concepts::arithmetic T>
    Vector3<T> Vector3<T>::up = Vector3<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));

    template<jate::utils::concepts::arithmetic T>
    Vector3<T> Vector3<T>::left = Vector3<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));

    template<jate::utils::concepts::arithmetic T>
    Vector3<T> Vector3<T>::back = Vector3<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

    // Aliases
    using Vector3i = Vector3<int32_t>;
    using Vector3ui = Vector3<uint32_t>;
    using Vector3f = Vector3<float>;
    using Vector3d = Vector3<double>;
}

#endif