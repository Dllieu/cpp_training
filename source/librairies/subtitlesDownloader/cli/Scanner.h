#ifndef __SCANNER_H__
#define __SCANNER_H__

using namespace System;
using namespace System::Collections::Generic;

namespace sdwrapper
{
    // Scanner
    // 
    public ref class Scanner
    {
    public:
        Scanner( unsigned numberOfSourceToList );

        // should pass a callback to not be blocking
        // from a given filename, retrieve possible source where we could download the subtitle from
        List< System::String^ >^ scan( System::String^ filename );

    private:
        unsigned     numberOfSourceToList_;
    };
}

#endif /* !__SCANNER_H__ */
