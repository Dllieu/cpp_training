//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __DESIGNPATTERN_VISITOR_H__
#define __DESIGNPATTERN_VISITOR_H__

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
    virtual void    visit(const Visitable& visitable) = 0;
};

template <class Visitable>
inline bool     genericVisit(AbstractVisitor& visitor, const Visitable& visitable)
{
    if (auto realVisitor = dynamic_cast< AbstractAcyclicVisitor<Visitable>* >(&visitor))
    {
        realVisitor->visit(visitable);
        return true;
    }
    return false;
}

template <typename RealVisitable>
class AbstractVisitable
{
public:
    inline bool    accept(AbstractVisitor& visitor) const
    {
        return genericVisit(visitor, static_cast<const RealVisitable&>(*this));
    }

private:
    virtual ~AbstractVisitable(){}; // prevent explit use of AbstractVisitable making destructor private
    friend RealVisitable;
}; 

// Generic Lambda Visitor
template <typename T>
struct function_traits : public function_traits<decltype( &T::operator() )>
{
    // NOTHING
};

template <typename ClassType, typename ReturnType, typename Arg>
struct function_traits< ReturnType( ClassType::* )( Arg ) const >
{
    using argumentType = typename std::decay< Arg >::type;
};

template <typename...> struct VariadicVisitor;

template <typename T>
class VariadicVisitor<T> : public AbstractAcyclicVisitor< typename function_traits< T >::argumentType >
{
public:
    VariadicVisitor(T&& t)
        : t_(std::move(t))
    {
        // NOTHING
    }

    inline void visit(const typename function_traits< T >::argumentType& args) override { t_(args); }

protected:
    T t_;
};

template <typename T, typename... Ts>
class VariadicVisitor<T, Ts...> : public VariadicVisitor<T>,
                                  public VariadicVisitor<Ts...>
{
public:
    VariadicVisitor(T&& t, Ts&&... ts)
        : VariadicVisitor<T>(std::forward<T>(t))
        , VariadicVisitor<Ts...>(std::forward<Ts>(ts)...)
    {
        // NOTHING
    }
};

template <typename... Ts>
inline VariadicVisitor<Ts...> makeVariadicVisitor(Ts&&... ts)
{
    return VariadicVisitor<Ts...>(std::forward<Ts>(ts)...);
}

}

#endif /* ! __DESIGNPATTERN_VISITOR_H__ */
