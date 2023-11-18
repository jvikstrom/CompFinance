#include "AADNumber.h"

//  Traditional AAD implementation of chapter 10
//  (With multi-dimensional additions of chapter 14)

//  The custom number type

#include <algorithm>
#include "AADTape.h"
#include "gaussians.h"

//	Access node (friends only)
//  Note const incorectness
Node &Number::node() const
{

#ifdef _DEBUG

    //  Help identify errors when arguments are not on tape

    //	Find node on tape
    auto it = tape->find(myNode);

    //	Not found
    if (it == tape->end())
    {
        throw runtime_error("Put a breakpoint here");
    }

#endif
    //  Const incorrectness
    return const_cast<Node &>(*myNode);
}

//	Convenient access to node data for friends

double &Number::derivative() { return myNode->pDerivatives[0]; }
double &Number::lDer() { return myNode->pDerivatives[0]; }
double &Number::rDer() { return myNode->pDerivatives[1]; }

double *&Number::adjPtr() { return myNode->pAdjPtrs[0]; }
double *&Number::leftAdj() { return myNode->pAdjPtrs[0]; }
double *&Number::rightAdj() { return myNode->pAdjPtrs[1]; }

//	Private constructors for operator overloading

//	Unary
Number::Number(Node &arg, const double val) : myValue(val)
{
    createNode<1>();

    myNode->pAdjPtrs[0] = Tape::multi
                              ? arg.pAdjoints
                              : &arg.mAdjoint;
}

//	Binary
Number::Number(Node &lhs, Node &rhs, const double val) : myValue(val)
{
    createNode<2>();

    if (Tape::multi)
    {
        myNode->pAdjPtrs[0] = lhs.pAdjoints;
        myNode->pAdjPtrs[1] = rhs.pAdjoints;
    }
    else
    {
        myNode->pAdjPtrs[0] = &lhs.mAdjoint;
        myNode->pAdjPtrs[1] = &rhs.mAdjoint;
    }
}

Number::Number() {}

//  Put on tape on construction
Number::Number(const double val) : myValue(val)
{
    createNode<0>();
}

//  Put on tape on assignment
Number &Number::operator=(const double val)
{
    myValue = val;
    createNode<0>();

    return *this;
}

//  Explicitly put existing Number on tape
void Number::putOnTape()
{
    createNode<0>();
}

//  Explicit coversion to double
Number::operator double &() { return myValue; }
Number::operator double() const { return myValue; }

//  Accessors: value and adjoint

double &Number::value()
{
    return myValue;
}
double Number::value() const
{
    return myValue;
}
//  Single dimensional
double &Number::adjoint()
{
    return myNode->adjoint();
}
double Number::adjoint() const
{
    return myNode->adjoint();
}
//  Multi dimensional
double &Number::adjoint(const size_t n)
{
    return myNode->adjoint(n);
}
double Number::adjoint(const size_t n) const
{
    return myNode->adjoint(n);
}

//  Reset all adjoints on the tape
//		note we don't use this method
void Number::resetAdjoints()
{
    tape->resetAdjoints();
}

//  Propagation

//  Propagate adjoints
//      from and to both INCLUSIVE
void Number::propagateAdjoints(
    Tape::iterator propagateFrom,
    Tape::iterator propagateTo)
{
    auto it = propagateFrom;
    while (it != propagateTo)
    {
        it->propagateOne();
        --it;
    }
    it->propagateOne();
}

//  Convenient overloads

//  Set the adjoint on this node to 1,
//  Then propagate from the node
void Number::propagateAdjoints(
    //  We start on this number's node
    Tape::iterator propagateTo)
{
    //  Set this adjoint to 1
    adjoint() = 1.0;
    //  Find node on tape
    auto propagateFrom = tape->find(myNode);
    propagateAdjoints(propagateFrom, propagateTo);
}

//  These 2 set the adjoint to 1 on this node
void Number::propagateToStart()
{
    propagateAdjoints(tape->begin());
}
void Number::propagateToMark()
{
    propagateAdjoints(tape->markIt());
}

//  This one only propagates
//  Note: propagation starts at mark - 1
void Number::propagateMarkToStart()
{
    propagateAdjoints(prev(tape->markIt()), tape->begin());
}

//  Multi dimensional case:
//  Propagate adjoints from and to both INCLUSIVE
void Number::propagateAdjointsMulti(
    Tape::iterator propagateFrom,
    Tape::iterator propagateTo)
{
    auto it = propagateFrom;
    while (it != propagateTo)
    {
        it->propagateAll();
        --it;
    }
    it->propagateAll();
}

//  Operator overloading

Number operator+(const Number &lhs, const Number &rhs)
{
    const double e = lhs.value() + rhs.value();
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), rhs.node(), e);
    //  Eagerly compute derivatives
    result.lDer() = 1.0;
    result.rDer() = 1.0;

    return result;
}
Number operator+(const Number &lhs, const double &rhs)
{
    const double e = lhs.value() + rhs;
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = 1.0;

    return result;
}
Number operator+(const double &lhs, const Number &rhs)
{
    return rhs + lhs;
}

Number operator-(const Number &lhs, const Number &rhs)
{
    const double e = lhs.value() - rhs.value();
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), rhs.node(), e);
    //  Eagerly compute derivatives
    result.lDer() = 1.0;
    result.rDer() = -1.0;

    return result;
}
Number operator-(const Number &lhs, const double &rhs)
{
    const double e = lhs.value() - rhs;
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = 1.0;

    return result;
}
Number operator-(const double &lhs, const Number &rhs)
{
    const double e = lhs - rhs.value();
    //  Eagerly evaluate and put on tape
    Number result(rhs.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = -1.0;

    return result;
}

Number operator*(const Number &lhs, const Number &rhs)
{
    const double e = lhs.value() * rhs.value();
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), rhs.node(), e);
    //  Eagerly compute derivatives
    result.lDer() = rhs.value();
    result.rDer() = lhs.value();

    return result;
}
Number operator*(const Number &lhs, const double &rhs)
{
    const double e = lhs.value() * rhs;
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = rhs;

    return result;
}
Number operator*(const double &lhs, const Number &rhs)
{
    return rhs * lhs;
}

Number operator/(const Number &lhs, const Number &rhs)
{
    const double e = lhs.value() / rhs.value();
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), rhs.node(), e);
    //  Eagerly compute derivatives
    const double invRhs = 1.0 / rhs.value();
    result.lDer() = invRhs;
    result.rDer() = -lhs.value() * invRhs * invRhs;

    return result;
}
Number operator/(const Number &lhs, const double &rhs)
{
    const double e = lhs.value() / rhs;
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = 1.0 / rhs;

    return result;
}
Number operator/(const double &lhs, const Number &rhs)
{
    const double e = lhs / rhs.value();
    //  Eagerly evaluate and put on tape
    Number result(rhs.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = -lhs / rhs.value() / rhs.value();

    return result;
}

Number pow(const Number &lhs, const Number &rhs)
{
    const double e = pow(lhs.value(), rhs.value());
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), rhs.node(), e);
    //  Eagerly compute derivatives
    result.lDer() = rhs.value() * e / lhs.value();
    result.rDer() = log(lhs.value()) * e;

    return result;
}
Number pow(const Number &lhs, const double &rhs)
{
    const double e = pow(lhs.value(), rhs);
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = rhs * e / lhs.value();

    return result;
}
Number pow(const double &lhs, const Number &rhs)
{
    const double e = pow(lhs, rhs.value());
    //  Eagerly evaluate and put on tape
    Number result(rhs.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = log(lhs) * e;

    return result;
}

Number max(const Number &lhs, const Number &rhs)
{
    const bool lmax = lhs.value() > rhs.value();
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), rhs.node(), lmax ? lhs.value() : rhs.value());
    //  Eagerly compute derivatives
    if (lmax)
    {
        result.lDer() = 1.0;
        result.rDer() = 0.0;
    }
    else
    {
        result.lDer() = 0.0;
        result.rDer() = 1.0;
    }

    return result;
}
Number max(const Number &lhs, const double &rhs)
{
    const bool lmax = lhs.value() > rhs;
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), lmax ? lhs.value() : rhs);
    //  Eagerly compute derivatives
    result.derivative() = lmax ? 1.0 : 0.0;

    return result;
}
Number max(const double &lhs, const Number &rhs)
{
    const bool rmax = rhs.value() > lhs;
    //  Eagerly evaluate and put on tape
    Number result(rhs.node(), rmax ? rhs.value() : lhs);
    //  Eagerly compute derivatives
    result.derivative() = rmax ? 1.0 : 0.0;

    return result;
}

Number min(const Number &lhs, const Number &rhs)
{
    const bool lmin = lhs.value() < rhs.value();
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), rhs.node(), lmin ? lhs.value() : rhs.value());
    //  Eagerly compute derivatives
    if (lmin)
    {
        result.lDer() = 1.0;
        result.rDer() = 0.0;
    }
    else
    {
        result.lDer() = 0.0;
        result.rDer() = 1.0;
    }

    return result;
}
Number min(const Number &lhs, const double &rhs)
{
    const bool lmin = lhs.value() < rhs;
    //  Eagerly evaluate and put on tape
    Number result(lhs.node(), lmin ? lhs.value() : rhs);
    //  Eagerly compute derivatives
    result.derivative() = lmin ? 1.0 : 0.0;

    return result;
}
Number min(const double &lhs, const Number &rhs)
{
    const bool rmin = rhs.value() < lhs;
    //  Eagerly evaluate and put on tape
    Number result(rhs.node(), rmin ? rhs.value() : lhs);
    //  Eagerly compute derivatives
    result.derivative() = rmin ? 1.0 : 0.0;

    return result;
}

Number &Number::operator+=(const Number &arg)
{
    *this = *this + arg;
    return *this;
}
Number &Number::operator+=(const double &arg)
{
    *this = *this + arg;
    return *this;
}

Number &Number::operator-=(const Number &arg)
{
    *this = *this - arg;
    return *this;
}
Number &Number::operator-=(const double &arg)
{
    *this = *this - arg;
    return *this;
}

Number &Number::operator*=(const Number &arg)
{
    *this = *this * arg;
    return *this;
}
Number &Number::operator*=(const double &arg)
{
    *this = *this * arg;
    return *this;
}

Number &Number::operator/=(const Number &arg)
{
    *this = *this / arg;
    return *this;
}
Number &Number::operator/=(const double &arg)
{
    *this = *this / arg;
    return *this;
}

//  Unary +/-
Number Number::operator-() const
{
    return 0.0 - *this;
}
Number Number::operator+() const
{
    return *this;
}

//  Overloading continued, unary functions

Number exp(const Number &arg)
{
    const double e = exp(arg.value());
    //  Eagerly evaluate and put on tape
    Number result(arg.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = e;

    return result;
}

Number log(const Number &arg)
{
    const double e = log(arg.value());
    //  Eagerly evaluate and put on tape
    Number result(arg.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = 1.0 / arg.value();

    return result;
}

Number sqrt(const Number &arg)
{
    const double e = sqrt(arg.value());
    //  Eagerly evaluate and put on tape
    Number result(arg.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = 0.5 / e;

    return result;
}

Number fabs(const Number &arg)
{
    const double e = fabs(arg.value());
    //  Eagerly evaluate and put on tape
    Number result(arg.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = arg.value() > 0.0 ? 1.0 : -1.0;

    return result;
}

Number normalDens(const Number &arg)
{
    const double e = normalDens(arg.value());
    //  Eagerly evaluate and put on tape
    Number result(arg.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = -arg.value() * e;

    return result;
}

Number normalCdf(const Number &arg)
{
    const double e = normalCdf(arg.value());
    //  Eagerly evaluate and put on tape
    Number result(arg.node(), e);
    //  Eagerly compute derivatives
    result.derivative() = normalDens(arg.value());

    return result;
}

//  Finally, comparison

bool operator==(const Number &lhs, const Number &rhs)
{
    return lhs.value() == rhs.value();
}
bool operator==(const Number &lhs, const double &rhs)
{
    return lhs.value() == rhs;
}
bool operator==(const double &lhs, const Number &rhs)
{
    return lhs == rhs.value();
}

bool operator!=(const Number &lhs, const Number &rhs)
{
    return lhs.value() != rhs.value();
}
bool operator!=(const Number &lhs, const double &rhs)
{
    return lhs.value() != rhs;
}
bool operator!=(const double &lhs, const Number &rhs)
{
    return lhs != rhs.value();
}

bool operator<(const Number &lhs, const Number &rhs)
{
    return lhs.value() < rhs.value();
}
bool operator<(const Number &lhs, const double &rhs)
{
    return lhs.value() < rhs;
}
bool operator<(const double &lhs, const Number &rhs)
{
    return lhs < rhs.value();
}

bool operator>(const Number &lhs, const Number &rhs)
{
    return lhs.value() > rhs.value();
}
bool operator>(const Number &lhs, const double &rhs)
{
    return lhs.value() > rhs;
}
bool operator>(const double &lhs, const Number &rhs)
{
    return lhs > rhs.value();
}

bool operator<=(const Number &lhs, const Number &rhs)
{
    return lhs.value() <= rhs.value();
}
bool operator<=(const Number &lhs, const double &rhs)
{
    return lhs.value() <= rhs;
}
bool operator<=(const double &lhs, const Number &rhs)
{
    return lhs <= rhs.value();
}

bool operator>=(const Number &lhs, const Number &rhs)
{
    return lhs.value() >= rhs.value();
}
bool operator>=(const Number &lhs, const double &rhs)
{
    return lhs.value() >= rhs;
}
bool operator>=(const double &lhs, const Number &rhs)
{
    return lhs >= rhs.value();
}
