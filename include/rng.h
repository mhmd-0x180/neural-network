#include <types.hpp>
#ifndef RNG_H
#define RNG_H

#include <stdint.h>

class Rng
{

    u64 state;

    u64 next()
    {
        u64 z = (state += 0x9E3779B97F4A7C15);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
        return z ^ (z >> 31);
    }

public:
    void seed(u64 seed)
    {
        state = seed;
    }


    u64 randInt(u64 min, u64 max)
    {
        return min + (u64)(next() % (max - min + 1));
    }

    f32 randFloat(f32 min, f32 max)
    {
        return min + ((next() >> 11) * (1.0f / 9007199254740991.0f)) * (max - min);
    }
    f64 randDouble(f64 min, f64 max)
    {
        return min + ((next() >> 11) * (1.0 / 9007199254740991.0)) * (max - min);
    }
};
#endif
