#ifndef __DESIGNPATTERN_VISITOR_H__
#define __DESIGNPATTERN_VISITOR_H__

namespace designpattern
{

class AbstractVisitor
{
public:
    virtual ~AbstractVisitor() {};
};

// public virtual : calling the destructor could be ambigous otherwise (compile error)
template <class Visitable>
class AbstractAcyclicVisitor : public virtual AbstractVisitor
{
public:
    virtual void    visit( const Visitable& visitable ) = 0;
};

template <class Visitable>
inline bool     genericVisit( AbstractVisitor& visitor, const Visitable* visitable )
{
    if ( visitable )
        if ( AbstractAcyclicVisitor<Visitable>* realVisitor = dynamic_cast< AbstractAcyclicVisitor<Visitable>* >( & visitor ) )
        {
            realVisitor->visit( *visitable );
            return true;
        }
    return false;
}

}

#endif /* ! __DESIGNPATTERN_VISITOR_H__ */
