#ifndef _MARKABLE_H
#define _MARKABLE_H

#include <cstdio>
#include <cstdlib>
#include <climits>   // defines  INT_MAX
#include <cassert>
#include <iostream>

// ==========================================================
// a single global "mark" can be used to remembed what part of 
// the model has been traversed.

class Markable {

public:

  Markable() { myMark = 0; }
  virtual ~Markable() {}

  int isMarked() { 
    if (myMark == currentMark) return 1;
    if (myMark > currentMark) {
      std::cout << "ACK!  myMark < currentMark" << std::endl;
      std::cout << myMark << " " << currentMark << std::endl;
    }
    assert (myMark < currentMark); 
    return 0; }

  int isMarkedSince(unsigned int mark) { 
    assert (myMark <= currentMark);
    assert (mark <= currentMark);
    if (myMark >= mark) return 1;
    return 0; }

  static void NextMark() { 
    if (currentMark >= INT_MAX) {
      printf ("ERROR:  currentMark rollover!\n");
      exit(1); }
    currentMark++; }

  static unsigned int GetCurrentMark() { return currentMark; }

  void Mark() {
    if (myMark >= currentMark)
      printf ("problem in Mark():  %d %d %d\n",(int)myMark,(int)currentMark,(int)INT_MAX);
    assert (!isMarked());
    assert (myMark < currentMark);
    myMark = currentMark; }

private:

  static unsigned int currentMark;
  unsigned int myMark;

};

// ==========================================================

#endif
