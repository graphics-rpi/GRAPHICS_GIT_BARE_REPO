#ifndef _HASH_H_
#define _HASH_H_

#include <vector>
#include "utils.h"


#ifdef __APPLE__
#if APPLE_MAVERICKS  // special define in CMakeLists.txt
#include <unordered_map>
#else
#include <ext/hash_map>
#endif
#else
#ifdef __CYGWIN__
#include <ext/hash_map>
#else
#include <unordered_map>
#endif
#endif

class Edge;
class Element;
class Spring;

//using namespace __gnu_cxx;

inline unsigned int ordered_two_hash(unsigned int a, unsigned int b) {
  return LARGE_PRIME_A * a + LARGE_PRIME_B * b;
}

inline unsigned int unordered_two_hash(unsigned int a, unsigned int b) {
  //cout << "u t h " << a << " " << b << endl;
  //assert (a != b);
  if (b < a) {
    unsigned int c = a;
    a = b;
    b = c;
  }
  //assert (a < b);
  return ordered_two_hash(a,b); // LARGE_PRIME_A * a + LARGE_PRIME_B * b;
}

struct orderedintpairhash {
  size_t operator()(std::pair<int,int> p) const {
    return ordered_two_hash(p.first,p.second);
  }
};

struct unorderedintpairhash {
  size_t operator()(std::pair<int,int> p) const {
    return unordered_two_hash(p.first,p.second);
  }
};

struct orderedsameintpair {
  bool operator()(std::pair<int,int> p1, std::pair<int,int>p2) const {
    if (p1.first == p2.first && p1.second == p2.second)
      return true;
    return false;
  }
};

struct unorderedsameintpair {
  bool operator()(std::pair<int,int> p1, std::pair<int,int>p2) const {
    if ((p1.first == p2.first && p1.second == p2.second) ||
	(p1.first == p2.second && p1.second == p2.first)) return true;
    return false;
  }
};

struct idhash {
  size_t operator()(unsigned int id) const {
    return LARGE_PRIME_A * id;
  }
};

struct sameid {
  bool operator()(unsigned int a, unsigned int b) const {
    if (a == b)
      return true;
    return false;
  }
};


#ifdef __APPLE__
#if APPLE_MAVERICKS
typedef std::unordered_map<std::pair<int,int>,int,unorderedintpairhash,unorderedsameintpair> vphashtype;
typedef std::unordered_map<std::pair<int,int>,std::vector<Edge*>,orderedintpairhash,orderedsameintpair> edgeshashtype;
typedef std::unordered_map<unsigned int,Element*,idhash,sameid> elementshashtype;
#else
typedef __gnu_cxx::hash_map<std::pair<int,int>,int,unorderedintpairhash,unorderedsameintpair> vphashtype;
typedef __gnu_cxx::hash_map<std::pair<int,int>,std::vector<Edge*>,orderedintpairhash,orderedsameintpair> edgeshashtype;
typedef __gnu_cxx::hash_map<unsigned int,Element*,idhash,sameid> elementshashtype;
#endif
#else
#ifdef __CYGWIN__
typedef __gnu_cxx::hash_map<std::pair<int,int>,int,unorderedintpairhash,unorderedsameintpair> vphashtype;
typedef __gnu_cxx::hash_map<std::pair<int,int>,std::vector<Edge*>,orderedintpairhash,orderedsameintpair> edgeshashtype;
typedef __gnu_cxx::hash_map<unsigned int,Element*,idhash,sameid> elementshashtype;
#else
typedef std::unordered_map<std::pair<int,int>,int,unorderedintpairhash,unorderedsameintpair> vphashtype;
typedef std::unordered_map<std::pair<int,int>,std::vector<Edge*>,orderedintpairhash,orderedsameintpair> edgeshashtype;
typedef std::unordered_map<unsigned int,Element*,idhash,sameid> elementshashtype;
#endif
#endif

#endif
