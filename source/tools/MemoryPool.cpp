//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include "MemoryPool.h"

using namespace tools;

MemoryPool::MemoryPool( size_t unitNumber /*= 50*/, size_t unitSize /*= 1024*/ )
    : unitSize_( unitSize )
    , blockSize_( unitNumber * ( sizeof( MemoryPool::Unit ) + unitSize ) )
    , memoryBlock_( new char[ blockSize_ ] )
    , allocatedBlock_( 0 )
    , freedBlock_( 0 )
{
    char* realMemoryBlock = memoryBlock_.get();
    size_t unitRealSize = unitSize_ + sizeof( MemoryPool::Unit );
    for ( size_t i = 0; i < unitNumber; ++i )
    {
        auto currentUnit = ( MemoryPool::Unit* )( realMemoryBlock + i * unitRealSize );

        currentUnit->previous = 0;
        if ( currentUnit->next = freedBlock_ )
            freedBlock_->previous = currentUnit;

        freedBlock_ = currentUnit;
    }
}

void*   MemoryPool::malloc( size_t requestedSize )
{
    if ( requestedSize > unitSize_ || ! memoryBlock_ || ! freedBlock_ )
        return malloc( requestedSize );

    auto currentUnit = freedBlock_;
    if ( freedBlock_ = currentUnit->next )
        freedBlock_->previous = 0;

    if ( currentUnit->next = allocatedBlock_ )
        allocatedBlock_->previous = currentUnit;
    allocatedBlock_ = currentUnit;

    // (char*) otherwise size of unitPtr + 4
    return (char*)currentUnit + sizeof( MemoryPool::Unit );
}

void    MemoryPool::free( void* p )
{
    char*   realMemoryBlock = memoryBlock_.get();
    if ( realMemoryBlock >= p || p >= ( realMemoryBlock + blockSize_ ) )
    {
        free( p );
        return;
    }

    auto currentUnit = (MemoryPool::Unit*)( (char*)p - sizeof( MemoryPool::Unit ) );
    if ( allocatedBlock_ = currentUnit->next )
        allocatedBlock_->previous = 0;

    if ( currentUnit->next = freedBlock_ )
        freedBlock_->previous = currentUnit;

    freedBlock_ = currentUnit;
}
