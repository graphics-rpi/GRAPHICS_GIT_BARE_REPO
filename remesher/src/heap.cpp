#include <cstdio>
#include <cstdlib>
#include "heap.h"

typedef Heapable* Heapable_Ptr;

Heap::Heap() {
  count = 0;
  capacity = INITIAL_CAPACITY;
  heap = new Heapable_Ptr[capacity]; 
}

Heap::~Heap() {
  delete [] heap;
}

Heapable* Heap::RemoveNext() {
  if (count < 1) return NULL;
  Heapable* max = heap[0];
  setHeapEntry(0,heap[count-1]);
  count--;
  downHeap(0);
  return max; 
}

Heapable* Heap::PeekNext() {
  if (count < 1) return NULL;
  Heapable *max = heap[0];
  return max; 
}

void Heap::Add(Heapable *h) {
  // resize, if necessary
  if (count+1 >= capacity) {
    capacity = capacity*2;
    Heapable** new_heap = new Heapable_Ptr[capacity];
    for (int i=0; i<count; i++)
      new_heap[i] = heap[i];
    delete [] heap;
    heap = new_heap;
  }
  count++;
  setHeapEntry(count-1,h);
  upHeap(count-1);
}

void Heap::Remove(Heapable *h) {
  int pos = h->getHeapPosition();
  assert (pos >= 0 && pos < count);
  assert (h == heap[pos]);
  setHeapEntry(pos,heap[count-1]);
  count--;
  if (pos < count) {
    // the replacing entry could need to be moved up OR down
    downHeap(pos);
    upHeap(pos);
  }
  h->setHeapPosition(-1);
}

void Heap::AddIfNotMember(Heapable *h) {
  int pos = h->getHeapPosition();
  if (pos < 0) {
    //assert (!this->Member(h));
    this->Add(h);
  } else {
    //assert (this->Member(h));
  }
}

void Heap::RemoveIfMember(Heapable *h) {
  int pos = h->getHeapPosition();
  if (pos < 0) {
    //assert (!this->Member(h));
  } else {
    //assert (this->Member(h));
    this->Remove(h);
  }
}

int Heap::Member(Heapable *h) {
  int pos = h->getHeapPosition();
  if (pos < 0) return 0;
  assert (h->Same(heap[pos]));
  return 1;
}

void Heap::upHeap(int i) {
  while (i > 0 &&
	 heap[parent(i)]->getHeapValue() < heap[i]->getHeapValue()) {
    swapHeapEntries(i,parent(i));
    i = parent(i);
  }
}

void Heap::downHeap(int i) {
  while (i < count) {
    int max = i;
    if (left(i) < count && 
	heap[left(i)]->getHeapValue() > heap[max]->getHeapValue())
      max = left(i);
    if (right(i) < count && 
	heap[right(i)]->getHeapValue() > heap[max]->getHeapValue())
      max = right(i);
    if (max == i) return;
    swapHeapEntries(i,max);
    i = max;
  }
}

void Heap::reHeap(int i) {
  int p_correct = heap[i]->getHeapValue() <= heap[parent(i)]->getHeapValue();
  int l_correct = left(i) >= count ||
    heap[i]->getHeapValue() >= heap[left(i)]->getHeapValue();
  int r_correct = right(i) >= count ||
    heap[i]->getHeapValue() >= heap[right(i)]->getHeapValue();
  assert (p_correct || (l_correct && r_correct));
  if (!p_correct) {
    assert (l_correct && r_correct);
    upHeap(i);
  }
  else if (!(l_correct && r_correct)) {
    downHeap(i);
  }
}

void Heap::Check() {
  int x = 0;
  while (x < count) {
    int l = left(x);
    int r = right(x);
    assert (heap[x]->getHeapPosition() == x);
    if ((l < count && heap[x]->getHeapValue() < heap[l]->getHeapValue()) ||
        (r < count && heap[x]->getHeapValue() < heap[r]->getHeapValue())) {
      assert(0);
    }
    x++;    
  }
}

void Heap::Print(const char *s) {
  for (int i = 0; i < count && i < 10; i++) {
    int p = parent(i);
    printf ("[%d][%d] ",i,p);
    heap[i]->Print();
  }
  Check();
}

