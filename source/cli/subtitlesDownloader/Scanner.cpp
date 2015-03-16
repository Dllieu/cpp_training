//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include "Scanner.h"

#include <algorithm>

using namespace sdwrapper;

Scanner::Scanner( unsigned numberOfSourceToList )
    : numberOfSourceToList_( std::max< unsigned >( 1, numberOfSourceToList ) )
{
    // NOTHING
}

List< String^ >^ Scanner::scan( String^ filename )
{
    List< String^ >^ result = gcnew List< String^ >();
    if (filename == nullptr)
        return result;

    result->Add( gcnew String( "uuu" ) );
    return result;
}
