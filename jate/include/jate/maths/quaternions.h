#ifndef Jate_Quaternion_H
#define Jate_Quaternion_H

#include <concepts>
#include <jate/maths/vectors.h>

namespace jate::maths
{
    template <std::floating_point T>
    struct Quaternion
    {
        T x, y, z, w;

        Quaternion(T xComp, T yComp, T zComp, T wComp) : x(xComp), y(yComp), z(zComp), w(wComp) {}

        Quaternion<T> normalized() const
        {
            T length_inv = 1.0 / (x * x + y * y + z * z + w * w);
            return Quaternion<T>(x * length_inv, y * length_inv, z * length_inv, w * length_inv);
        }

        static Quaternion<T> fromAxisAngle(Vector3<T> axis, T angle)
        {
            T sinHalf = sin(angle / 2.0);

            return Quaternion<T>(
                axis.x * sinHalf,
                axis.y * sinHalf,
                axis.z * sinHalf,
                cos(angle / 2.0)
            ).normalized();
        }

        static Quaternion<T> identity;
    };

    template <std::floating_point T>
    Quaternion<T> Quaternion<T>::identity = Quaternion<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));

    using Quaternionf = Quaternion<float>;
    using Quaterniond = Quaternion<double>;
}

#endif