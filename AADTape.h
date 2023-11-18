
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

//  AAD implementation of chapter 10
//  (With multi-dimensional additions of chapter 14)

//  Implementation of the Tape

//  Unchanged for AADET of chapter 15

#include "blocklist.h"
#include "AADNode.h"

constexpr size_t BLOCKSIZE = 16384; //	Number of nodes
constexpr size_t ADJSIZE = 32768;   //	Number of adjoints
constexpr size_t DATASIZE = 65536;  //	Data in bytes

class Tape
{
    //	Working with multiple results / adjoints?
    static bool multi;

    //  Storage for adjoints in multi-dimensional case (chapter 14)
    blocklist<double, ADJSIZE> myAdjointsMulti;

    //  Storage for derivatives and child adjoint pointers
    blocklist<double, DATASIZE> myDers;
    blocklist<double *, DATASIZE> myArgPtrs;

    //  Storage for the nodes
    blocklist<Node, BLOCKSIZE> myNodes;

    //	Padding so tapes in a vector don't interfere
    char myPad[64];

    friend auto setNumResultsForAAD(const bool, const size_t);
    friend struct numResultsResetterForAAD;
    friend class Number;

public:
    //  Build note in place and return a pointer
    //	N : number of childs (arguments)
    template <size_t N>
    Node *recordNode()
    {
        //  Construct the node in place on tape
        Node *node = myNodes.emplace_back(N);

        //  Store and zero the adjoint(s)
        if (multi)
        {
            node->pAdjoints = myAdjointsMulti.emplace_back_multi(Node::numAdj);
            fill(node->pAdjoints, node->pAdjoints + Node::numAdj, 0.0);
        }

        //	Store the derivatives and child adjoint pointers unless leaf
        if constexpr (N > 0)
        {
            node->pDerivatives = myDers.emplace_back_multi<N>();
            node->pAdjPtrs = myArgPtrs.emplace_back_multi<N>();
        }

        return node;
    }

    //  Reset all adjoints to 0
    void resetAdjoints();

    //  Clear
    void clear();

    //  Rewind
    void rewind();
    //  Set mark
    void mark();

    //  Rewind to mark
    void rewindToMark();

    //  Iterators

    using iterator = blocklist<Node, BLOCKSIZE>::iterator;

    iterator begin();

    iterator end();

    iterator markIt();

    iterator find(Node *node);
};