
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

//  Traditional AAD implementation of chapter 10
//  (With multi-dimensional additions of chapter 14)

//  The custom number type

#include "AADNode.h"
#include "AADTape.h"
#include <algorithm>

class Number {
  //  Value and node are the only data members
  double myValue;
  Node *myNode;

  //  Create node on tape
  template <size_t N> void createNode() { myNode = tape->recordNode<N>(); }

  //	Access node (friends only)
  //  Note const incorectness
  Node &node() const;

  //	Convenient access to node data for friends

  double &derivative();
  double &lDer();
  double &rDer();

  double *&adjPtr();
  double *&leftAdj();
  double *&rightAdj();

  //	Private constructors for operator overloading

  //	Unary
  Number(Node &arg, const double val);

  //	Binary
  Number(Node &lhs, Node &rhs, const double val);

public:
  //  Static access to tape
  static thread_local Tape *tape;

  //  Public constructors for leaves

  Number();

  //  Put on tape on construction
  explicit Number(const double val);

  //  Put on tape on assignment
  Number &operator=(const double val);

  //  Explicitly put existing Number on tape
  void putOnTape();

  //  Explicit coversion to double
  explicit operator double &();
  explicit operator double() const;

  //  Accessors: value and adjoint

  double &value();
  double value() const;
  //  Single dimensional
  double &adjoint();
  double adjoint() const;
  //  Multi dimensional
  double &adjoint(const size_t n);
  double adjoint(const size_t n) const;

  //  Reset all adjoints on the tape
  //		note we don't use this method
  void resetAdjoints();

  //  Propagation

  //  Propagate adjoints
  //      from and to both INCLUSIVE
  static void propagateAdjoints(Tape::iterator propagateFrom,
                                Tape::iterator propagateTo);

  //  Convenient overloads

  //  Set the adjoint on this node to 1,
  //  Then propagate from the node
  void propagateAdjoints(
      //  We start on this number's node
      Tape::iterator propagateTo);

  //  These 2 set the adjoint to 1 on this node
  void propagateToStart();
  void propagateToMark();

  //  This one only propagates
  //  Note: propagation starts at mark - 1
  static void propagateMarkToStart();

  //  Multi dimensional case:
  //  Propagate adjoints from and to both INCLUSIVE
  static void propagateAdjointsMulti(Tape::iterator propagateFrom,
                                     Tape::iterator propagateTo);

  //  Operator overloading

  inline friend Number operator+(const Number &lhs, const Number &rhs);
  inline friend Number operator+(const Number &lhs, const double &rhs);
  inline friend Number operator+(const double &lhs, const Number &rhs);
  inline friend Number operator-(const Number &lhs, const Number &rhs);
  inline friend Number operator-(const Number &lhs, const double &rhs);
  inline friend Number operator-(const double &lhs, const Number &rhs);

  inline friend Number operator*(const Number &lhs, const Number &rhs);
  inline friend Number operator*(const Number &lhs, const double &rhs);
  inline friend Number operator*(const double &lhs, const Number &rhs);

  inline friend Number operator/(const Number &lhs, const Number &rhs);
  inline friend Number operator/(const Number &lhs, const double &rhs);
  inline friend Number operator/(const double &lhs, const Number &rhs);

  inline friend Number pow(const Number &lhs, const Number &rhs);
  inline friend Number pow(const Number &lhs, const double &rhs);
  inline friend Number pow(const double &lhs, const Number &rhs);

  inline friend Number max(const Number &lhs, const Number &rhs);
  inline friend Number max(const Number &lhs, const double &rhs);
  inline friend Number max(const double &lhs, const Number &rhs);

  inline friend Number min(const Number &lhs, const Number &rhs);
  inline friend Number min(const Number &lhs, const double &rhs);
  inline friend Number min(const double &lhs, const Number &rhs);

  Number &operator+=(const Number &arg);
  Number &operator+=(const double &arg);

  Number &operator-=(const Number &arg);
  Number &operator-=(const double &arg);

  Number &operator*=(const Number &arg);
  Number &operator*=(const double &arg);

  Number &operator/=(const Number &arg);
  Number &operator/=(const double &arg);

  //  Unary +/-
  Number operator-() const;
  Number operator+() const;

  //  Overloading continued, unary functions

  inline friend Number exp(const Number &arg);

  inline friend Number log(const Number &arg);

  inline friend Number sqrt(const Number &arg);

  inline friend Number fabs(const Number &arg);

  inline friend Number normalDens(const Number &arg);
  inline friend Number normalCdf(const Number &arg);

  //  Finally, comparison

  inline friend bool operator==(const Number &lhs, const Number &rhs);
  inline friend bool operator==(const Number &lhs, const double &rhs);
  inline friend bool operator==(const double &lhs, const Number &rhs);

  inline friend bool operator!=(const Number &lhs, const Number &rhs);
  inline friend bool operator!=(const Number &lhs, const double &rhs);
  inline friend bool operator!=(const double &lhs, const Number &rhs);

  inline friend bool operator<(const Number &lhs, const Number &rhs);
  inline friend bool operator<(const Number &lhs, const double &rhs);
  inline friend bool operator<(const double &lhs, const Number &rhs);

  inline friend bool operator>(const Number &lhs, const Number &rhs);
  inline friend bool operator>(const Number &lhs, const double &rhs);
  inline friend bool operator>(const double &lhs, const Number &rhs);

  inline friend bool operator<=(const Number &lhs, const Number &rhs);
  inline friend bool operator<=(const Number &lhs, const double &rhs);
  inline friend bool operator<=(const double &lhs, const Number &rhs);

  inline friend bool operator>=(const Number &lhs, const Number &rhs);
  inline friend bool operator>=(const Number &lhs, const double &rhs);
  inline friend bool operator>=(const double &lhs, const Number &rhs);
};
