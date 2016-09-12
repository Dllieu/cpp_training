#include "Split.h"

using namespace tools;

namespace
{
    template < typename T >
    std::vector< T > split_impl( const T& text, const std::string& separators )
    {
        std::vector< T > tokens;
        std::size_t start = 0, end = 0;

        while ( ( end = text.find_first_of( separators, start ) ) != std::string::npos )
        {
            tokens.emplace_back( text.substr( start, end - start ) );
            start = end + 1;
        }
        tokens.emplace_back( text.substr( start ) );
        return tokens;
    }
}

std::vector< std::string > split( const std::string& text, const std::string& separators )
{
    return split_impl( text, separators );
}

//std::vector< std::string_view > SplitString::to_string_views( const std::string_view& text, const std::string& separators )
//{
//    return split_impl( text, separators );
//}
