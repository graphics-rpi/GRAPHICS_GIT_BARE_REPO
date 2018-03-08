#ifndef _SCORE_H_
#define _SCORE_H_


#include <map>
#include <string>
#include <iostream>
#include <fstream>

class Score {
public:
  Score() { good = neutral = bad = 0; }
  /*
    friend std::ostream& operator<<(std::ostream& ostr, const Score& s) {
    ostr << "(good:" << s.good << " neutral:" << s.neutral << " bad:" << s.bad << ")";
    return ostr;
  }
  */
  void incrGood()    { good++; }
  void incrNeutral() { neutral++; }
  void incrBad()     { bad++; }

  int totalMoves() const { return good + neutral + bad; }
  int getGood() const { return good; }
  int getNeutral() const { return neutral; }
  int getBad() const { return bad; }

private:
  int good;
  int neutral;
  int bad;
};



inline void LoadHighScores(const std::string &high_scores_filename,
                           const std::string &image_filename,
                           int cols, int rows,
                           int &fastest_time, int &fewest_moves, int &num_people) {
  
  std::ifstream istr(high_scores_filename.c_str());
  if (!istr) { std::cout << "warning: high scores file does not exist (yet)" << std::endl;
    return;
  }
  assert (istr);

  fastest_time = -1;
  fewest_moves = -1;
  num_people = -1;

  std::string img_filename, token;
  int _cols,_rows,time,num_moves,num_peeps;
  while (1) {
    if (!(istr >> img_filename)) break;
    istr >> token;
    assert (token == "cols:");
    istr >> _cols;
    assert (_cols > 0);
    istr >> token;
    assert (token == "rows:");
    istr >> _rows;    
    assert (_rows > 0);
    istr >> token;
    assert (token == "time:");
    istr >> time;
    if (time == 0) {
      std::cout << "time should not == 0!" << std::endl;
      time = 1;
    }
    assert (time > 0);
    istr >> token;
    assert (token == "num_moves:");
    istr >> num_moves;
    if (num_moves == 0) {
      std::cout << "num moves should not == 0!" << std::endl;
      num_moves = 1;
    }
    assert (num_moves > 0);
    istr >> token;
    assert (token == "num_people:");
    istr >> num_peeps;
    if (num_peeps == 0) {
      std::cout << "num people should not == 0!" << std::endl;
      num_peeps = 1;
    }
    assert (num_peeps > 0);
    if (img_filename == image_filename &&
        rows == _rows && cols == _cols) {
      if (fastest_time < 0 || fastest_time > time) {
        fastest_time = time;
        fewest_moves = num_moves;
        num_people = num_peeps;
      }
    }
  }
}

inline void AppendHighScore(const std::string &high_scores_filename,
                            const std::string &image_filename,
                            int cols, int rows,
                            int time, const std::map<int,Score> &all_scores) {
  std::ofstream ostr(high_scores_filename.c_str(),std::ios_base::app);
  assert (ostr);

  int num_peeps = all_scores.size();
  int num_moves = 0;
  for (std::map<int,Score>::const_iterator itr = all_scores.begin();
       itr != all_scores.end(); itr++) {
    num_moves += itr->second.totalMoves();
  }
  ostr << image_filename << "  cols: " << cols << "  rows: " << rows 
       << "  time: " << time << "  num_moves: " << num_moves << "  num_people: " << num_peeps << std::endl;
}


#endif
