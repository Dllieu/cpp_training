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
