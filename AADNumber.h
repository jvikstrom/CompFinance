
/*
Written by Antoine Savine in 2018

This code is the strict IP of Antoine Savine

License to use and alter this code for personal and commercial applications
is freely granted to any person or company who purchased a copy of the book

Modern Computational Finance: AAD and Parallel Simulations
Antoine Savine
Wiley, 2018

As long as this comment is preserved at the top of the file
*/

#pragma once

#include <algorithm>
#include "AADTape.h"

class Number
{
    double myValue;
    Node* myNode;

    //  Node creation on tape

    template <class NodeType>
    inline void createNode()
    {
        //  Placement syntax to allocate in place on tape
        myNode = new (tape->allocate<sizeof(NodeType)>()) NodeType;
    }

    //  Access to node for friends
    template <class NodeType = Node>
    inline NodeType* node() const
    {
        return static_cast<NodeType*>(myNode);
    }

    //  To help overloading
    static const struct LeafType {} leaf;
    static const struct UnaryType {} unary;
    static const struct BinaryType {} binary;

    Number(const double val, UnaryType) :
        myValue(val)
    {
        createNode<UnaryNode>();
    }

    Number(const double val, BinaryType) :
        myValue(val)
    {
        createNode<BinaryNode>();
    }

public:

    //  Static access to tape
    static thread_local Tape* tape;

    //  Constructors

    Number() {}

    explicit Number(const double val) :
        myValue(val)
    {
        createNode<Leaf>();
    }

    //  Assignments

    Number& operator=(const double val)
    {
        myValue = val;
        createNode<Leaf>();

        return *this;
    }

    //  Put on tape
    void putOnTape()
    {
        createNode<Leaf>();
    }

    //  Accessors: value and adjoint

    double& value()
    {
        return myValue;
    }
    double value() const
    {
        return myValue;
    }
    double& adjoint()
    {
        return myNode->adjoint;
    }
    double adjoint() const
    {
        return myNode->adjoint;
    }

    //  Propagation

    //  Reset all adjoints on the tape
    static void resetAdjoints()
    {
        for (Node& node : *tape) node.adjoint = 0.0;
    }
    //  Propagate adjoints
    //      from and to both INCLUSIVE
    static void propagateAdjoints(
        Tape::iterator propagateFrom,
        Tape::iterator propagateTo)
    {
        auto it = propagateFrom;
        while (it != propagateTo)
        {
            it->propagate();
            --it;
        }
        it->propagate();
    }

    //  Convenient overloads

    //  Set the adjoint on this node to 1,
    //  Then propagate from the node
    void propagateAdjoints(
        //  We start on this number's node
        Tape::iterator propagateTo,
        //  reset adjoints first?
        const bool reset = false)
    {
        //  Reset
        if (reset) resetAdjoints();
        //  Set this adjoint to 1
        adjoint() = 1.0;
        //  Find node on tape
        auto it = tape->find(myNode);
        //  Reverse and propagate until we hit the stop
        while (it != propagateTo)
        {
            it->propagate();
            --it;
        }
        it->propagate();
    }

    //  These 2 set the adjoint to 1 on this node
    void propagateToStart(
        const bool reset = false)
    {
        propagateAdjoints(tape->begin(), reset);
    }
    void propagateToMark(
        const bool reset = false)
    {
        propagateAdjoints(tape->markIt(), reset);
    }

    //  This one only propagates
    //  Note: propagation starts at mark - 1
    static void propagateMarkToStart()
    {
        propagateAdjoints(--tape->markIt(), tape->begin());
    }

    //  Operator overloading

    inline friend Number operator+(const Number& lhs, const Number& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs.value() + rhs.value(), binary);
        //  Set arguments
        result.node<BinaryNode>()->arguments[0] = lhs.node();
        result.node<BinaryNode>()->arguments[1] = rhs.node();
        //  Eagerly compute derivatives
        result.node<BinaryNode>()->derivatives[0] = 1.0;
        result.node<BinaryNode>()->derivatives[1] = 1.0;

        return result;
    }
    inline friend Number operator+(const Number& lhs, const double& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs.value() + rhs, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = lhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = 1.0;

        return result;

    }
    inline friend Number operator+(const double& lhs, const Number& rhs)
    {
        return rhs + lhs;
    }

    inline friend Number operator-(const Number& lhs, const Number& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs.value() - rhs.value(), binary);
        //  Set arguments
        result.node<BinaryNode>()->arguments[0] = lhs.node();
        result.node<BinaryNode>()->arguments[1] = rhs.node();
        //  Eagerly compute derivatives
        result.node<BinaryNode>()->derivatives[0] = 1.0;
        result.node<BinaryNode>()->derivatives[1] = -1.0;

        return result;
    }
    inline friend Number operator-(const Number& lhs, const double& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs.value() - rhs, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = lhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = 1.0;

        return result;

    }
    inline friend Number operator-(const double& lhs, const Number& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs - rhs.value(), unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = rhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = -1.0;

        return result;
    }

    inline friend Number operator*(const Number& lhs, const Number& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs.value() * rhs.value(), binary);
        //  Set arguments
        result.node<BinaryNode>()->arguments[0] = lhs.node();
        result.node<BinaryNode>()->arguments[1] = rhs.node();
        //  Eagerly compute derivatives
        result.node<BinaryNode>()->derivatives[0] = rhs.value();
        result.node<BinaryNode>()->derivatives[1] = lhs.value();

        return result;
    }
    inline friend Number operator*(const Number& lhs, const double& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs.value() * rhs, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = lhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = rhs;

        return result;

    }
    inline friend Number operator*(const double& lhs, const Number& rhs)
    {
        return rhs * lhs;
    }

    inline friend Number operator/(const Number& lhs, const Number& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs.value() / rhs.value(), binary);
        //  Set arguments
        result.node<BinaryNode>()->arguments[0] = lhs.node();
        result.node<BinaryNode>()->arguments[1] = rhs.node();
        //  Eagerly compute derivatives
        const double invRhs = 1.0 / rhs.value();
        result.node<BinaryNode>()->derivatives[0] = 1.0 * invRhs;
        result.node<BinaryNode>()->derivatives[1] = -lhs.value() * invRhs * invRhs;

        return result;
    }
    inline friend Number operator/(const Number& lhs, const double& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs.value() / rhs, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = lhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = 1.0 / rhs;

        return result;

    }
    inline friend Number operator/(const double& lhs, const Number& rhs)
    {
        //  Eagerly evaluate and put on tape
        Number result(lhs / rhs.value(), unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = rhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = -lhs / rhs.value() / rhs.value();

        return result;
    }

    Number& operator+=(const Number& arg)
    {
        *this = *this + arg;
        return *this;
    }
    Number& operator+=(const double& arg)
    {
        *this = *this + arg;
        return *this;
    }

    Number& operator-=(const Number& arg)
    {
        *this = *this - arg;
        return *this;
    }
    Number& operator-=(const double& arg)
    {
        *this = *this - arg;
        return *this;
    }

    Number& operator*=(const Number& arg)
    {
        *this = *this * arg;
        return *this;
    }
    Number& operator*=(const double& arg)
    {
        *this = *this * arg;
        return *this;
    }

    Number& operator/=(const Number& arg)
    {
        *this = *this / arg;
        return *this;
    }
    Number& operator/=(const double& arg)
    {
        *this = *this / arg;
        return *this;
    }

    //  Unary +/-
    Number operator-() const
    {
        return 0.0 - *this;
    }
    Number operator+() const 
    {
        return *this;
    }

    //  Unary functions
    inline friend Number exp(const Number& arg)
    {
        const double e = exp(arg.value());
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = arg.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = e;

        return result;
    }

    inline friend Number log(const Number& arg)
    {
        //  Eagerly evaluate and put on tape
        Number result(log(arg.value()), unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = arg.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = 1.0 / arg.value();

        return result;
    }

    inline friend Number sqrt(const Number& arg)
    {
        const double e = sqrt(arg.value());
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = arg.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = 0.5 / e;

        return result;
    }

    inline friend Number fabs(const Number& arg)
    {
        const double e = fabs(arg.value());
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = arg.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = arg.value() > 0.0 ? 1.0 : -1.0;

        return result;
    }

    inline friend Number normalDens(const Number& arg)
    {
        const double e = normalDens(arg.value());
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = arg.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = - arg.value() * e;

        return result;
    }

    inline friend Number normalCdf(const Number& arg)
    {
        const double e = normalCdf(arg.value());
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = arg.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = normalDens(arg.value());

        return result;
    }

    //  Binary functions
    inline friend Number pow(const Number& lhs, const Number& rhs)
    {
        const double e = pow(lhs.value(), rhs.value());
        //  Eagerly evaluate and put on tape
        Number result(e, binary);
        //  Set arguments
        result.node<BinaryNode>()->arguments[0] = lhs.node();
        result.node<BinaryNode>()->arguments[1] = rhs.node();
        //  Eagerly compute derivatives
        result.node<BinaryNode>()->derivatives[0] = rhs.value() * e / lhs.value();
        result.node<BinaryNode>()->derivatives[1] = log(lhs.value()) * e;

        return result;
    }
    inline friend Number pow(const Number& lhs, const double& rhs)
    {
        const double e = pow(lhs.value(), rhs);
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = lhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = rhs * e / lhs.value();

        return result;
    }
    inline friend Number pow(const double& lhs, const Number& rhs)
    {
        const double e = pow(lhs, rhs.value());
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = rhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = log(lhs) * e;

        return result;
    }

    inline friend Number max(const Number& lhs, const Number& rhs)
    {
        const double e = max(lhs.value(), rhs.value());
        //  Eagerly evaluate and put on tape
        Number result(e, binary);
        //  Set arguments
        result.node<BinaryNode>()->arguments[0] = lhs.node();
        result.node<BinaryNode>()->arguments[1] = rhs.node();
        //  Eagerly compute derivatives
        if (lhs.value() > rhs.value())
        {
            result.node<BinaryNode>()->derivatives[0] = 1.0;
            result.node<BinaryNode>()->derivatives[1] = 0.0;
        }
        else
        {
            result.node<BinaryNode>()->derivatives[0] = 0.0;
            result.node<BinaryNode>()->derivatives[1] = 1.0;
        }

        return result;
    }
    inline friend Number max(const Number& lhs, const double& rhs)
    {
        const double e = max(lhs.value(), rhs);
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = lhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = lhs.value() > rhs? 1.0 : 0.0;

        return result;
    }
    inline friend Number max(const double& lhs, const Number& rhs)
    {
        const double e = max(lhs, rhs.value());
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = rhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = rhs.value() > lhs ? 1.0 : 0.0;

        return result;
    }

    inline friend Number min(const Number& lhs, const Number& rhs)
    {
        const double e = min(lhs.value(), rhs.value());
        //  Eagerly evaluate and put on tape
        Number result(e, binary);
        //  Set arguments
        result.node<BinaryNode>()->arguments[0] = lhs.node();
        result.node<BinaryNode>()->arguments[1] = rhs.node();
        //  Eagerly compute derivatives
        if (lhs.value() < rhs.value())
        {
            result.node<BinaryNode>()->derivatives[0] = 1.0;
            result.node<BinaryNode>()->derivatives[1] = 0.0;
        }
        else
        {
            result.node<BinaryNode>()->derivatives[0] = 0.0;
            result.node<BinaryNode>()->derivatives[1] = 1.0;
        }

        return result;
    }
    inline friend Number min(const Number& lhs, const double& rhs)
    {
        const double e = min(lhs.value(), rhs);
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = lhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = lhs.value() < rhs ? 1.0 : 0.0;

        return result;
    }
    inline friend Number min(const double& lhs, const Number& rhs)
    {
        const double e = min(lhs, rhs.value());
        //  Eagerly evaluate and put on tape
        Number result(e, unary);
        //  Set arguments
        result.node<UnaryNode>()->argument = rhs.node();
        //  Eagerly compute derivatives
        result.node<UnaryNode>()->derivative = rhs.value() < lhs ? 1.0 : 0.0;

        return result;
    }

    //  Finally, comparison

    inline friend bool operator==(const Number& lhs, const Number& rhs)
    {
        return lhs.value() == rhs.value();
    }
    inline friend bool operator==(const Number& lhs, const double& rhs)
    {
        return lhs.value() == rhs;
    }
    inline friend bool operator==(const double& lhs, const Number& rhs)
    {
        return lhs == rhs.value();
    }

    inline friend bool operator!=(const Number& lhs, const Number& rhs)
    {
        return lhs.value() != rhs.value();
    }
    inline friend bool operator!=(const Number& lhs, const double& rhs)
    {
        return lhs.value() != rhs;
    }
    inline friend bool operator!=(const double& lhs, const Number& rhs)
    {
        return lhs != rhs.value();
    }

    inline friend bool operator<(const Number& lhs, const Number& rhs)
    {
        return lhs.value() < rhs.value();
    }
    inline friend bool operator<(const Number& lhs, const double& rhs)
    {
        return lhs.value() < rhs;
    }
    inline friend bool operator<(const double& lhs, const Number& rhs)
    {
        return lhs < rhs.value();
    }

    inline friend bool operator>(const Number& lhs, const Number& rhs)
    {
        return lhs.value() > rhs.value();
    }
    inline friend bool operator>(const Number& lhs, const double& rhs)
    {
        return lhs.value() > rhs;
    }
    inline friend bool operator>(const double& lhs, const Number& rhs)
    {
        return lhs > rhs.value();
    }

    inline friend bool operator<=(const Number& lhs, const Number& rhs)
    {
        return lhs.value() <= rhs.value();
    }
    inline friend bool operator<=(const Number& lhs, const double& rhs)
    {
        return lhs.value() <= rhs;
    }
    inline friend bool operator<=(const double& lhs, const Number& rhs)
    {
        return lhs <= rhs.value();
    }

    inline friend bool operator>=(const Number& lhs, const Number& rhs)
    {
        return lhs.value() >= rhs.value();
    }
    inline friend bool operator>=(const Number& lhs, const double& rhs)
    {
        return lhs.value() >= rhs;
    }
    inline friend bool operator>=(const double& lhs, const Number& rhs)
    {
        return lhs >= rhs.value();
    }
};


