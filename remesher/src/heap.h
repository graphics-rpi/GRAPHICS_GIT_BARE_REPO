#ifndef _HEAP_H
#define _HEAP_H

#include <cstdio>
#include <cassert>

// Heap (Priority Queue) of Heapable Objects
// implementation from CLR chapter 7

#define INITIAL_CAPACITY 1000

class Heap;

// =============================================================
// Heapable things can be stored in a Heap (Priority Queue)

class Heapable {

public:

  Heapable() { heapPosition = -1; }
  virtual ~Heapable() {}

  // accessor
  virtual double getHeapValue() = 0;

protected:

  virtual int Same(Heapable *h) { 
    if (this == h) return 1; return 0; }
  virtual void Print(const char *s = "") = 0;

  int getHeapPosition() { return heapPosition; }
  void setHeapPosition(int p) { heapPosition = p; }

  friend class Heap;
    
private:
  int heapPosition;

};

// =============================================================

class Heap {

public:

  // ------------------------
  // CONSTRUCTOR & DESTRUCTOR
  Heap();
  ~Heap();

  // ---------
  // ACCESSORS
  int Empty() const { if (count < 1) return 1; return 0; }
  int Member(Heapable *h);
  int Count() const { return count; }

  // HACK:  use only for checking the heap... in EdgeHold::Check 
  // should rewrite to make an iterator...  ugh
  Heapable* GrabEntry(int i) {
    assert (i >= 0 && i < count);
    return heap[i];
  }

  // ---------
  // MODIFIERS
  void Add(Heapable *h);
  void Remove(Heapable *h);
  void AddIfNotMember(Heapable *h);
  void RemoveIfMember(Heapable *h);
  Heapable* RemoveNext();
  Heapable* PeekNext();

  //private:

  void reHeap(Heapable *h) {
    int pos = h->getHeapPosition();
    assert (pos >= 0);
    if (heap[pos] != h) {
      printf ("WHOOPS!  %d\n", pos);
    }
    assert (heap[pos] == h);
    this->reHeap(pos); }
  void upHeap(Heapable *h) {
    int pos = h->getHeapPosition();
    assert (heap[pos] == h);
    assert (pos >= 0);
    this->upHeap(pos); }
  void downHeap() {
    this->downHeap(0); }
  
  void Check();

public:
  void Print(const char *s = "");
  //  void THING();

private:

  // -----------------------------
  // PRIVATE ACCESSORS & MODIFIERS
  static int parent(int i) { return (i-1)/2; }
  static int left(int i) { return 2*i + 1; }
  static int right(int i) { return 2*i + 2; }

  void setHeapEntry(int i, Heapable *h) {
    assert (i >=0 && i < count);
    assert (h != NULL);
    heap[i] = h;
    h->setHeapPosition(i); }
  void swapHeapEntries(int i, int j) {
    assert (i >=0 && i < count);
    assert (j >=0 && j < count);    
    Heapable *tmp = heap[i];
    setHeapEntry(i,heap[j]);
    setHeapEntry(j,tmp); }

  void upHeap(int i);
  void downHeap(int i);
  void reHeap(int i);

  // --------------
  // REPRESENTATION
  Heapable** heap;
  int count;
  int capacity;

};

#endif
