#pragma once

#include <vector>
#include <string>

namespace tools
{
    std::vector< std::string >       split( const std::string& text, const std::string& separators );
    //std::vector< std::string_view >  split( const std::string_view& text, const std::string& separators );
}
