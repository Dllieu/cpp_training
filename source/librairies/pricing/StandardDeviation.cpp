#include "StandardDeviation.h"

#include <numeric>
#include <cmath>

using namespace pricing;

// Population vs. Sample
//
// The primary task of inferential statistics (or estimating or forecasting) is making an opinion about something by using only an incomplete sample of data. 
// In statistics it is very important to distinguish between population and sample. A population is defined as all members (e.g. occurrences, prices, annual returns) of a specified group. Population is the whole group.
// A sample is a part of a population that is used to describe the characteristics (e.g. mean or standard deviation) of the whole population. The size of a sample can be less than 1%, or 10%, or 60% of the population, but it is never the whole population.

// PopulationStandardDeviation
double StandardDeviation::operator()( const std::vector< double >& points ) const
{
    if ( points.empty() )
        return 0;

    double mean = std::accumulate( std::begin( points ), std::end( points ), 0.0 ) / points.size();
    double variance = std::accumulate( std::begin( points ), std::end( points ), 0.0, [ mean ]( double pastResult, double point ){ return pastResult + std::pow( point - mean, 2 ); } ) / points.size();
    return std::sqrt( variance );
}
