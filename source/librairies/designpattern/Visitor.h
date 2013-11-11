#ifndef __DESIGNPATTERN_VISITOR_H__
#define __DESIGNPATTERN_VISITOR_H__

#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/cat.hpp>
#include <functional>

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

// Generic Lambda Visitor
template <typename T>
struct function_traits : public function_traits<decltype( &T::operator() )>
{
    // NOTHING
};

template <typename ClassType, typename ReturnType, typename Arg>
struct function_traits< ReturnType( ClassType::* )( Arg ) const >
{
    typedef ReturnType resultType;
    typedef typename std::decay< Arg >::type argumentType;
};

#define VISITOR_INHERITANCE( z, n, gen ) gen< typename function_traits< L##n >::argumentType >
#define MEMBER_DECL( z, n, nil ) L##n _l##n;
#define MEMBER_ASSIGN( z, n, nil ) _l##n( std::move( l##n ) )
#define VISIT_DECL( z, n, nil ) virtual void visit( const typename function_traits< L##n >::argumentType &f ) override { _l##n( f ); }
#define FORMAL_VISITOR_PARAM( z, n, nil ) L##n&& l##n
#define EFFECTIVE_VISITOR_PARAM( z, n, nil ) std::forward< L##n >( l##n )
#define DECAY_TYPE( z, n, nil ) typename std::decay< L##n >::type

#define MAKE_VISITOR_1( n, gen ) \
    template < BOOST_PP_ENUM_PARAMS( n, typename L ) > \
    struct BOOST_PP_CAT( Visitor, n ) : BOOST_PP_ENUM( n, VISITOR_INHERITANCE, gen ) \
    {\
    BOOST_PP_REPEAT( n, MEMBER_DECL, ~ ) \
    BOOST_PP_CAT( Visitor, n ) ( BOOST_PP_ENUM_BINARY_PARAMS( n, L, l ) ) : BOOST_PP_ENUM( n, MEMBER_ASSIGN, ~ ) {} \
    BOOST_PP_REPEAT( n, VISIT_DECL, ~ ) \
    };\
    \
    template < BOOST_PP_ENUM_PARAMS( n, typename L ) > \
    BOOST_PP_CAT( Visitor, n ) < BOOST_PP_ENUM( n, DECAY_TYPE, ~ ) > \
    makeVisitor( BOOST_PP_ENUM( n, FORMAL_VISITOR_PARAM, ~ ) ) \
    {\
        return BOOST_PP_CAT( Visitor, n ) < BOOST_PP_ENUM( n, DECAY_TYPE, ~ ) > ( BOOST_PP_ENUM( n, EFFECTIVE_VISITOR_PARAM, ~ ) ); \
    }

#define MAKE_VISITOR( z, n, gen ) MAKE_VISITOR_1( BOOST_PP_INC( n ), gen )
BOOST_PP_REPEAT( 5, MAKE_VISITOR, AbstractAcyclicVisitor ) // tell how many VISITOR param you want to handle (from 1 to n)

#undef VISITOR_INHERITANCE
#undef MEMBER_DECL
#undef MEMBER_ASSIGN
#undef VISIT_DECL
#undef FORMAL_VISITOR_PARAM
#undef EFFECTIVE_VISITOR_PARAM
#undef DECAY_TYPE
#undef MAKE_VISITOR_1
#undef MAKE_VISITOR

//template <  typename L0 >
//struct Visitor1 :  AbstractAcyclicVisitor< typename function_traits< L0 >::argumentType >
//{
//    L0 _l0;
//    Visitor1 (  L0 l0 )
//        : _l0( std::move( l0 ) )
//    {}
//    
//    virtual void visit( const typename function_traits< L0 >::argumentType &f ) override
//    {
//        _l0( f );
//    }
//};
//
//template <  typename L0 >
//Visitor1 <  typename std::decay< L0 >::type >
//makeVisitor(  L0 && l0 )
//{
//    return Visitor1 <  typename std::decay< L0 >::type > (  std::forward< L0 >( l0 ) );
//}
//
//template <  typename L0 , typename L1 >
//struct Visitor2 :  AbstractAcyclicVisitor< typename function_traits< L0 >::argumentType >
//                ,  AbstractAcyclicVisitor< typename function_traits< L1 >::argumentType >
//{
//    L0 _l0;
//    L1 _l1;
//    
//    Visitor2 (  L0 l0 , L1 l1 )
//        :  _l0( std::move( l0 ) )
//        , _l1( std::move( l1 ) )
//    {}
//    
//    virtual void visit( const typename function_traits< L0 >::argumentType &f ) override { _l0( f ); }
//    virtual void visit( const typename function_traits< L1 >::argumentType &f ) override { _l1( f ); }
//};
//
//template <  typename L0 , typename L1 >
//Visitor2 <  typename std::decay< L0 >::type , typename std::decay< L1 >::type >
//makeVisitor(  L0 && l0 , L1 && l1 )
//{
//    return Visitor2 <  typename std::decay< L0 >::type , typename std::decay< L1 >::type > (  std::forward< L0 >( l0 ) , std::forward< L1 >( l1 ) );
//}
//
//// ...

}

#endif /* ! __DESIGNPATTERN_VISITOR_H__ */
