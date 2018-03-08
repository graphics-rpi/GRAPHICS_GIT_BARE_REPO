#include "../paint/button.h"
#include <vector>
#include <iostream>
#include <stdlib.h>

class Score;

class PuzzlePiece : public Button {

public:
  PuzzlePiece(const Pt &p, double w, double h, const std::string &f,
              double u0=0, double v0=0, double u1=1, double v1=1);
  virtual ~PuzzlePiece() {}
  
  bool GetBorderVal(int index);
  void SetBorderVal(int index, bool newval);
  void ResetBorder();
  void FullBorder();
  double CentroidDist(const PuzzlePiece &a);
  
  void paint(const Vec3f &background_color=Vec3f(0,0,0) ) const;
  
  Pt getCorrectPoint() { return correct_pt; }
  bool isCorrect() { return correct; }
  
  void setCorrectness(bool c) { correct = c;  /*if(c) std::cout << "set correctness " << c << std::endl;*/ }
  void setCorrectPoint(Pt p ) { correct_pt = p; }

  void OnPress();
  void OnRelease(Score &score);

private:
  void Fade(int index) const;
  void Init();
  
  // default versions ok
  PuzzlePiece(const PuzzlePiece &p) : ClickableObject(p), Button (p) { exit(0); }
  PuzzlePiece& operator=(const PuzzlePiece &p) { exit(0); }
  
  
  //Makes indexing the borders array a little less painful
  enum position{center=0, topleft=1, left=2, botleft=3, bot=4,
                botright=5, right=6, topright=7, top=8};
  
  
  // REPRESENTATION
  //The borders array holds a bool that says if the edge should be lit or not
  bool borders[9];
  double x0, y0, x1, y1;
  double x0p, y0p, x1p, y1p;
  double u0p, v0p, u1p, v1p;
  bool correct;
  Pt correct_pt;

  Pt atclick;
  
};


inline std::ostream& operator<<(std::ostream &ostr, const PuzzlePiece &p) {
  ostr << "PUZZLE PIECE " << p.getLastTouched() << std::endl;
  return ostr;
}

inline bool piece_sorter(const PuzzlePiece *p1, const PuzzlePiece *p2) { 
  assert (p1 != NULL);
  assert (p2 != NULL);
  return (*p1) < (*p2); 
}
