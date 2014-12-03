//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TOOLS_MEMORY_POOL_H__
#define __TOOLS_MEMORY_POOL_H__

#include <stddef.h>
#include <memory>

namespace tools
{

class MemoryPool
{
public:
    MemoryPool( size_t unitNumber = 50, size_t unitSize = 1024 );

    void*   malloc( size_t requestedSize );
    void    free( void* p );

private:
    size_t                          unitSize_;
    size_t                          blockSize_;

    std::unique_ptr<char[]>           memoryBlock_;

private:
    struct Unit
    {
        Unit *previous, *next;
    };

    Unit*                           allocatedBlock_;
    Unit*                           freedBlock_;
};

};

#endif /* ! __TOOLS_MEMORY_POOL_H__ */
