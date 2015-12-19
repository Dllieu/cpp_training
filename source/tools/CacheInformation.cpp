//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include "CacheInformation.h"

const char* tools::to_string( CacheSize cacheSize )
{
    switch ( cacheSize )
    {
        case CacheSize::L1:
            return "L1";

        case CacheSize::L2:
            return "L2";

        case CacheSize::L3:
            return "L3";

        default:
            return "DRAM";
    }
}
