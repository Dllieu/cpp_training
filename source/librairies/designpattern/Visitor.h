#ifndef __DESIGNPATTERN_VISITOR_H__
#define __DESIGNPATTERN_VISITOR_H__

namespace designpattern
{

class AbstractVisitor
{
public:
    virtual ~AbstractVisitor() {};
};

// Virtual inheritance ensures that the same base class appearing two or more times in an inheritance graph has all of its instances merged
// (which is the case for the RealVisitor which will implement several AbstractAcyclicVisitor (diamond inheritance case)
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
