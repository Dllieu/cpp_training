#ifndef __STANDARDDEVIATION_H__
#define __STANDARDDEVIATION_H__

#include <vector>

namespace pricing
{
    class StandardDeviation
    {
    public:
        static double compute( const std::vector< double >& points );
    };
}

#endif /* !__STANDARDDEVIATION_H__ */
