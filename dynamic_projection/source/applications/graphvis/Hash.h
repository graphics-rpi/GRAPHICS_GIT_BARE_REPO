#ifndef _HASH_H_
#define _HASH_H_

// to handle different platforms with different variants of a developing standard
// NOTE: You may need to adjust these depending on your installation

#ifdef __APPLE__
#include <ext/hash_map>
#else
#include <unordered_map>
#endif

/*
#elif defined(__CYGWIN__)
#include <ext/hash_map>
#elif defined(__LINUX__)
#include <unordered_map>
#elif defined(__FREEBSD__)
#include <ext/hash_map>
#elif defined(MACOSX)
#include <ext/hash_map>
#elif defined(LINUX)
#include <unordered_map>
#elif defined(UNIX)
#include <ext/hash_map>
#else
#error "unknown system"
#endif
*/

//#include <unordered_map>

class QTEdge;
class QTNode;
class Spring;

#define LARGE_PRIME_A 10007
#define LARGE_PRIME_B 11003


// ===================================================================================
// DIRECTED EDGES are stored in a hash table using a simple hash
// function based on the indices of the start and end vertices
// ===================================================================================

inline unsigned int ordered_two_int_hash(unsigned int a, unsigned int b) {
  return LARGE_PRIME_A * a + LARGE_PRIME_B * b;
}

struct orderedDrawnNodepairhash {
  size_t operator()(std::pair<QTNode*,QTNode*> p) const {
    if (p.second == NULL) {
      return ordered_two_int_hash(p.first->getID(),-1);
    } else {
      return ordered_two_int_hash(p.first->getID(),p.second->getID());
    }
  }
};

struct orderedsameDrawnNodepair {
  bool operator()(std::pair<QTNode*, QTNode*> p1, std::pair<QTNode*, QTNode*>p2) const {
    int s1id = -1;
    int s2id = -1;
    if (p1.second != NULL) s1id = p1.second->getID();
    if (p2.second != NULL) s2id = p2.second->getID();
    if (p1.first->getID() == p2.first->getID() && s1id == s2id) //p1.second->getID() == p2.second->getID())
      return true;
    return false;
  }
};


// to handle different platforms with different variants of a developing standard
// NOTE: You may need to adjust these depending on your installation
#ifdef __APPLE__
typedef __gnu_cxx::hash_map<std::pair<QTNode*,QTNode*>,QTEdge*,orderedDrawnNodepairhash,orderedsameDrawnNodepair> edgeshashtype;
//#elif defined(__CYGWIN__)
//typedef __gnu_cxx::hash_map<std::pair<DrawnNode*,DrawnNode*>,Edge*,orderedDrawnNodepairhash,orderedsameDrawnNodepair> edgeshashtype;
//#elif defined(__LINUX__)
#else
typedef std::unordered_map<std::pair<QTNode*,QTNode*>,QTEdge*,orderedDrawnNodepairhash,orderedsameDrawnNodepair> edgeshashtype;
//#elif defined(__FREEBSD__)
//typedef __gnu_cxx::hash_map<std::pair<DrawnNode*,DrawnNode*>,Edge*,orderedDrawnNodepairhash,orderedsameDrawnNodepair> edgeshashtype;
#endif

//typedef std::unordered_map<std::pair<DrawnNode*,DrawnNode*>,Edge*,orderedDrawnNodepairhash,orderedsameDrawnNodepair> edgeshashtype;

#endif // _HASH_H_
