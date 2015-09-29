//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

// need the double layer indirection for s1, s2 to be expanded to their respective values
// e.g. CONCATENATE( eg, __LINE__ ) : eg__LINE__, CONCATENATE_IMPL( eg, __LINE__ ) : eg687
#define ANONYMOUS_CONCATENATE_IMPL(s1, s2) s1##s2
#define ANONYMOUS_CONCATENATE(s1, s2) ANONYMOUS_CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) \
    ANONYMOUS_CONCATENATE( str, __COUNTER__ )
#else
    ANONYMOUS_CONCATENATE( str, __LINE__ )
#endif
