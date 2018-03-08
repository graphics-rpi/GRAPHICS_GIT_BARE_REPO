#ifndef __COLOR_H__
#define __COLOR_H__

#include <map>
#include <vector>

#include "../../../../remesher/src/vectors.h"
#include "MersenneTwister.h"

class Colors {
 public:
  Colors() {
    AddColor(Vec3f(1,0,0));
    AddColor(Vec3f(0,1,0));
    AddColor(Vec3f(0,0,1));
    AddColor(Vec3f(1,1,0));
    AddColor(Vec3f(0,1,1));
    AddColor(Vec3f(1,0,1));

    AddColor(Vec3f(0.5,0,0));
    AddColor(Vec3f(0,0.5,0));
    AddColor(Vec3f(0,0,0.5));
    AddColor(Vec3f(0.5,0.5,0));
    AddColor(Vec3f(0,0.5,0.5));
    AddColor(Vec3f(0.5,0,0.5));

    AddColor(Vec3f(1,0.5,0));
    AddColor(Vec3f(0,1,0.5));
    AddColor(Vec3f(0.5,0,1));

    AddColor(Vec3f(0.5,1,0));
    AddColor(Vec3f(0,0.5,1));
    AddColor(Vec3f(1,0,0.5));

    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    // AddColor(Vec3f(1,1,0));
    
    std::cout << "all colorssize" << all_colors.size() << std::endl;

    mtrand = MTRand(37); // deterministic randomness
  }

  Colors(const std::vector<Vec3f> &colors) {
    for (unsigned int i = 0; i < colors.size(); i++) {
      AddColor(colors[i]);
    }
  }

  void AddColor(const Vec3f &c) { all_colors.push_back(std::make_pair(c,0)); }

  void AssignColor(int id, unsigned int index) {
    std::map<int,int>::iterator itr = color_map.find(id);
    assert (itr == color_map.end());
    if (itr == color_map.end()) {
      assert (index < all_colors.size());
      all_colors[index].second++;
      color_map[id] = index;
    }
  }

  void ReleaseColor(int id) {
    std::map<int,int>::iterator itr = color_map.find(id);
    assert (itr != color_map.end());
    int old_index = itr->second;
    assert (all_colors[old_index].second > 0);
    all_colors[old_index].second--;
    color_map.erase(itr);
  }

  void ReAssignColor(int id, unsigned int new_index) {

    new_index = new_index % all_colors.size();

    std::cout << "reassign color map size" << color_map.size() << std::endl;

    std::map<int,int>::iterator itr = color_map.find(id);
        if (itr == color_map.end()) {
    std::cout << "WHY IS THIS A RE-ASSIGN COLOR? " << id << " SHOULD BE AN ASSIGN COLOR" << std::endl;
    AssignColor(id,new_index);
     return;
    }
    assert (itr != color_map.end());
    int old_index = itr->second;
    assert (all_colors[old_index].second > 0);
    all_colors[old_index].second--;
    assert (new_index < all_colors.size());
    all_colors[new_index].second++;
    color_map[id] = new_index;
  }

  void AssignRandomAvailableColor(int id) {
    int best_index = -1;
    int best_count = -1;
    int rand = mtrand.randInt(all_colors.size()-1);

    for (unsigned int i = 0; i < all_colors.size(); i++) {
      int index = (i + rand) % all_colors.size();
      int count = all_colors[index].second;
      if (count == 0) {
	AssignColor(id,index);
	return;
      }
      if (best_index == -1 || count < best_count) {
	best_index = index;
	best_count = count;
      }
    }
    std::cout << "WARNING: ALL COLORS USED" << std::endl;
    AssignColor(id,best_index);
  }
  void RemoveId(int id) {
    std::map<int,int>::iterator itr = color_map.find(id);
    assert (itr != color_map.end());
    int index = itr->second;
    assert (all_colors[index].second > 0);
    all_colors[index].second--;
    color_map.erase(itr);
  }

  const Vec3f& GetColor(int id) const {
    std::cout << "get color color map size" << color_map.size() << std::endl;

    std::map<int,int>::const_iterator itr = color_map.find(id);
    static Vec3f red(1,0,0);
    if (itr == color_map.end()) {
      std::cout << "COULDN't FIND COLOR, RETURNIGN RED" << itr->second << std::endl;
      //itr = color_map.begin();
      return red;
    }
    assert (itr != color_map.end());
    std::cout << "RETURNING A PRETTY NON RED COLOR" << itr->second << std::endl;
    return all_colors[itr->second].first;
  }

  int GetColorIndex(int id) const {
    std::map<int,int>::const_iterator itr = color_map.find(id);
    assert (itr != color_map.end());
    return itr->second;
  }

  Vec3f GetColorFromAllColors(int id)const{
    id %= all_colors.size();
    assert(id >= 0 && id < (int)all_colors.size());
    return all_colors[id].first;
  }

 private: 
  // REPRESENTATION
  std::map<int,int> color_map;  // id -> index in color vector
  std::vector<std::pair<Vec3f,int> > all_colors;

  MTRand mtrand;
};

#endif
