
#include "AADTape.h"

//  AAD implementation of chapter 10
//  (With multi-dimensional additions of chapter 14)

//  Implementation of the Tape

//  Unchanged for AADET of chapter 15

#include "blocklist.h"
#include "AADNode.h"

//  Reset all adjoints to 0
void Tape::resetAdjoints()
{
    if (multi)
    {
        myAdjointsMulti.memset(0);
    }
    else
    {
        for (Node &node : myNodes)
        {
            node.mAdjoint = 0;
        }
    }
}

//  Clear
void Tape::clear()
{
    myAdjointsMulti.clear();
    myDers.clear();
    myArgPtrs.clear();
    myNodes.clear();
}

//  Rewind
void Tape::rewind()
{

#ifdef _DEBUG

    //  In debug mode, always wipe
    //      makes it easier to identify errors

    clear();

#else
    //  In release mode, rewind and reuse

    if (multi)
    {
        myAdjointsMulti.rewind();
    }
    myDers.rewind();
    myArgPtrs.rewind();
    myNodes.rewind();

#endif
}

//  Set mark
void Tape::mark()
{
    if (multi)
    {
        myAdjointsMulti.setmark();
    }
    myDers.setmark();
    myArgPtrs.setmark();
    myNodes.setmark();
}

//  Rewind to mark
void Tape::rewindToMark()
{
    if (multi)
    {
        myAdjointsMulti.rewind_to_mark();
    }
    myDers.rewind_to_mark();
    myArgPtrs.rewind_to_mark();
    myNodes.rewind_to_mark();
}

//  Iterators

Tape::iterator Tape::begin()
{
    return myNodes.begin();
}

Tape::iterator Tape::end()
{
    return myNodes.end();
}

Tape::iterator Tape::markIt()
{
    return myNodes.mark();
}

Tape::iterator Tape::find(Node *node)
{
    return myNodes.find(node);
}
