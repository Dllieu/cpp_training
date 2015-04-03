//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __GENERICS_HASHCOMBINE_H__
#define __GENERICS_HASHCOMBINE_H__

#include <functional>

namespace generics
{
    // the Hash128to64 function from Google's cityhash (available under the MIT License).
    inline uint64_t Hash128to64(const uint64_t upper, const uint64_t lower)
    {
        // Murmur-inspired hashing.
        const uint64_t kMul = 0x9ddfea08eb382d69ULL;
        uint64_t a = (lower ^ upper) * kMul;
        a ^= (a >> 47);

        uint64_t b = (upper ^ a) * kMul;
        b ^= (b >> 47);
        b *= kMul;

        return b;
    }

    template <class Hash>
    size_t  hashCombineGeneric()
    {
        return 0;
    }

    // based on facebook folly
    template <class Hash, typename T, typename ...Ts>
    size_t  hashCombineGeneric(const T& t, const Ts&... ts)
    {
        size_t seed = Hash::hash(t);
        if (sizeof...(ts) == 0)
            return seed;

        // Not recursion, will create N templates
        auto remainder = hashCombineGeneric< Hash >(ts...);
        // ...
        return static_cast<size_t>( Hash128to64(seed, remainder) );
    }

    class StdHasher
    {
    public:
        template <typename T>
        static size_t hash(const T& t)
        {
            // At least string and pair might collide with std::hash
            return std::hash< T >()(t);
        }
    };

    template <typename T, typename ...Ts>
    size_t hashCombine(const T& t, const Ts&... ts)
    {
        return hashCombineGeneric< StdHasher >(t, ts...);
    }
}

#endif /* ! __GENERICS_HASHCOMBINE_H__ */
