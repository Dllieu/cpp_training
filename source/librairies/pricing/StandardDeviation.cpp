#include "StandardDeviation.h"

#include <numeric>
#include <cmath>

using namespace pricing;

/*static*/ double StandardDeviation::compute( const std::vector< double >& points )
{
    if ( points.empty() )
        return 0;

    double mean = std::accumulate( std::begin( points ), std::end( points ), 0.0 ) / points.size();
    double variance = std::accumulate( std::begin( points ), std::end( points ), 0.0, [ mean ]( double pastResult, double point ){ return pastResult + std::pow( point - mean, 2 ); } ) / points.size();
    return std::sqrt( variance );
}
