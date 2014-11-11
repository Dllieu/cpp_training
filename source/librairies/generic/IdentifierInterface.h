#ifndef __IDENTIFIERINTERFACE_H__
#define __IDENTIFIERINTERFACE_H__

#include <functional>
#include <sstream>

#include "designpattern/Visitor.h"

// No namespace
class IdentifierInterface
{
public:
    IdentifierInterface() {}
    virtual ~IdentifierInterface() {}

    virtual std::string         type() const = 0;
    virtual std::string         name() const = 0;
    virtual std::string         toUri() const = 0;

    virtual void                accept( designpattern::AbstractVisitor& v ) const = 0;

    inline std::string          toFullUri() const
    {
        std::ostringstream ss;
        ss << type() << ":" << toUri();
        return ss.str();
    }

protected:
    inline static void  copy( const IdentifierInterface* from, void* to )
    {
        if ( from )
            from->copyTo( to );
    }
    virtual void        copyTo( void* target ) const = 0;
};

inline std::ostream&    operator<<( std::ostream& os, const IdentifierInterface& id )
{
    return os << id.name();
}

namespace details
{
    template < typename T >
    struct Castor : designpattern::AbstractAcyclicVisitor< T >
    {
        Castor() : result( nullptr ) {}
        virtual void    visit( const T& t ) { result = &t };
        const T* result;
    };
}

template < typename T >
T   identifier_cast( const IdentifierInterface* id )
{
    if ( ! id )
        return nullptr;

    details::Castor< std::decay< T >::type >    cast;
    id->accept( cast );
    return static_cast< T >( cast.result );
}

// Throw unsafe
template < typename T >
T   identifier_cast( const IdentifierInterface& id )
{
    typedef typename std::remove_reference< T >::type const* constPtrT;
    auto result = identifier_cast< constPtrT >( &id );
    if ( ! result )
        throw std::bad_cast();
    return *result;
}

#endif /* ! __IDENTIFIERINTERFACE_H__ */
