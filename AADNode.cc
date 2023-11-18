#include "AADNode.h"

#include <algorithm>

using namespace std;

Node::Node(const size_t N) : n(N) {}

double &Node::adjoint() { return mAdjoint; }
//	multi
double &Node::adjoint(const size_t n) { return pAdjoints[n]; }

//  Back-propagate adjoints to arguments adjoints

//  Single case, chapter 10
void Node::propagateOne() {
  //  Nothing to propagate
  if (!n || !mAdjoint)
    return;

  for (size_t i = 0; i < n; ++i) {
    *(pAdjPtrs[i]) += pDerivatives[i] * mAdjoint;
  }
}

//  Multi case, chapter 14
void Node::propagateAll() {
  //  No adjoint to propagate
  if (!n || std::all_of(pAdjoints, pAdjoints + numAdj,
                        [](const double &x) { return !x; }))
    return;

  for (size_t i = 0; i < n; ++i) {
    double *adjPtrs = pAdjPtrs[i], ders = pDerivatives[i];

    //  Vectorized!
    for (size_t j = 0; j < numAdj; ++j) {
      adjPtrs[j] += ders * pAdjoints[j];
    }
  }
}
