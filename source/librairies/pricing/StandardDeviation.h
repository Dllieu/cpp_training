//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __STANDARDDEVIATION_H__
#define __STANDARDDEVIATION_H__

#include <vector>

namespace pricing
{
    class StandardDeviation
    {
    public:
        double operator()( const std::vector< double >& points ) const;
    };
}

#endif /* !__STANDARDDEVIATION_H__ */
