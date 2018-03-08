#ifndef _WORKCREW_H_
#define _WORKCREW_H_

#include <string>

class Workcrew {
public:

  Workcrew(std::string n, std::string t) 
    : name(n), service_type(t), assigned(false), location(""), time_until_completion(0) {
    assert (name != "");
    assert (service_type != "");
  }
  
  
  void Assign(std::string l, int t) {
    assert (assigned == false);
    assert (l != "");
    assert (t > 0);
    assigned = true;
    location = l;
    time_until_completion = t;
  }

  void Tick() {
    if (assigned == true) {
      assert (time_until_completion > 0);
      time_until_completion--;
      if (time_until_completion == 0) {
        assigned = false;
        location = "";
      }
    } else {
      assert (time_until_completion == 0);
      assert (location == "");
    }
  }

private:
  std::string name;
  std::string service_type;
  bool assigned;
  std::string location;
  int time_until_completion;

};

#endif
