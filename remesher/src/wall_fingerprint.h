
#ifndef _WALL_FINGERPRINT_H_
#define _WALL_FINGERPRINT_H_

class ArgParser;
#include <set>
#include <map>
#include <string>

class Poly;
class BasicWall;

enum WALL_FINGERPRINT { FINGERPRINT_NONE, FINGERPRINT_FRONT, FINGERPRINT_BACK, FINGERPRINT_MIDDLE };
enum ARRANGEMENT_CELL_TYPE { 
                             ACT_WALL, 
                             ACT_WINDOW_MAGENTA, ACT_WINDOW_YELLOW, ACT_WINDOW_CYAN, 
                             ACT_BOUNDARY, 

                             ACT_OTHER,  ACT_INTERIOR,  

                             ACT_OTHER_SKYLIGHT, ACT_INTERIOR_SKYLIGHT, 
                             ACT_OTHER_FURNITURE, ACT_INTERIOR_FURNITURE, 
                             ACT_OTHER_FURNITURE_SKYLIGHT, ACT_INTERIOR_FURNITURE_SKYLIGHT };






// ===============================================================================================
// ===============================================================================================
struct SubGroupData {
public:
SubGroupData() : area_sum(0), wall_area_sum(0), area_weighted_enclosure(0), interior2(false), mixed(false), inferredwall(false), tinyunusedwall(false) {}
  std::set<Poly*> polys;
  double area_sum;
  double wall_area_sum;
  double area_weighted_enclosure;
  bool interior2;
  bool mixed;
  bool inferredwall;
  bool tinyunusedwall;
};

struct ZoneData {
  ZoneData() {
    subgroup = -1;
    interior = false;
    average_enclosure = -1;
    area = -1;
  }
  std::vector<WALL_FINGERPRINT> print;
  int subgroup;
  std::map<int,double> wall_border;
  bool interior;
  double average_enclosure;
  double area;
};

struct WallEvidence {
  std::vector<WALL_FINGERPRINT> a;
  int which_subgroup_a;
  std::vector<WALL_FINGERPRINT> b;
  int which_subgroup_b;
  double wall_area;
  double nonwall_area;
};

struct PolygonGroupData {
public:
  //constructor
  PolygonGroupData() {}
  // accessor
  int numSubGroups() const { assert (!subgroups.empty()); return subgroups.size(); }
  bool IsInterior2(Poly *p) const { return IsInterior2(whichSubGroup(p)); }
  bool IsMixed(Poly *p) const { return IsMixed(whichSubGroup(p)); }
  bool IsInferredWall(Poly *p) const { return IsInferredWall(whichSubGroup(p)); }
  bool IsTinyUnusedWall(Poly *p) const { return IsTinyUnusedWall(whichSubGroup(p)); }
  bool IsInterior2(int i) const { return get(i).interior2; }
  bool IsMixed(int i) const { return get(i).mixed; }
  bool IsInferredWall(int i) const { return get(i).inferredwall; }
  bool IsTinyUnusedWall(int i) const { return get(i).tinyunusedwall; }
  double getArea(int i) const { return get(i).area_sum; }
  double getMaxDistance(int i) const;
  double IsRealWall(int i) const { 
    double frac = get(i).wall_area_sum / get(i).area_sum; 
    assert (frac < 0.001 || frac > 0.999);
    return (frac > 0.9); } 
  double getWeightedCentroidEnclosure(int i) { return get(i).area_weighted_enclosure / get(i).area_sum; }
  void AnalyzeEnclosureHistogram(int i, std::string &is_enclosed, double tuned_enclosure_threshold);
  void AverageEnclosure(int i, double &average_enclosure, double &area);
  ZoneData AnalyzeZone(const std::vector<WALL_FINGERPRINT> &print, int i);
  bool SplitSubGroup(int i);
  int whichSubGroup(Poly *p) const {
    assert (p != NULL);
    assert (!allpolys.empty());
    int num_subgroups = numSubGroups();
    for (int i = 0; i < num_subgroups; i++) {
      if (get(i).polys.find(p) != get(i).polys.end()) return i;
    }
    assert (0);
    return -1;
  }
  std::vector<int> whichSubGroupsAreNeighbors(int i, const PolygonGroupData &data2) const;

  // modifier
  void SetInterior(int i) { get(i).interior2 = true; get(i).inferredwall = false; get(i).tinyunusedwall = false; }
  void SetMixed(int i) { get(i).mixed = true; }
  void SetInferredWall(int i) { get(i).inferredwall = true; get(i).interior2 = false; get(i).tinyunusedwall = false; }
  void SetTinyUnusedWall(int i) { get(i).inferredwall = false; get(i).interior2 = false; get(i).tinyunusedwall = true; }

  void AddPolygon(Poly *p);
  void ConnectedGroupAnalysis();

  void Print() const;


  // HACK:THIS WAS PRIVATE BEFORE
  SubGroupData& get(int i) { 
    //cout << "get " << i << " " << subgroups.size() << endl; 
    assert (i >= 0 && i < (int)subgroups.size()); return subgroups[i]; }
  const SubGroupData& get(int i) const { 
    //cout << "get " << i << " " << subgroups.size() << endl; 
    assert (i >= 0 && i < (int)subgroups.size()); return subgroups[i]; }

private:
  // representation
  std::set<Poly*> allpolys;
  std::vector<SubGroupData> subgroups;



};

// ===============================================================================================
// ===============================================================================================

struct PolygonLabels {
  // accessors
  int numPrints() const { return groups.size(); }
  bool IsInterior2(const std::vector<WALL_FINGERPRINT> &print, Poly *p) const;
  bool IsMixed(const std::vector<WALL_FINGERPRINT> &print, Poly *p) const;
  bool IsInferredWall(const std::vector<WALL_FINGERPRINT> &print, Poly *p) const;
  bool IsTinyUnusedWall(const std::vector<WALL_FINGERPRINT> &print, Poly *p) const;
  bool IsInterior2(const std::vector<WALL_FINGERPRINT> &print, int i) const;
  bool IsInferredWall(const std::vector<WALL_FINGERPRINT> &print, int i) const;
  bool IsTinyUnusedWall(const std::vector<WALL_FINGERPRINT> &print, int i) const;
  // modifiers
  void Clear() { groups.clear(); }
  void SetInterior(const std::vector<WALL_FINGERPRINT> &print, int i); 
  void SetMixed(const std::vector<WALL_FINGERPRINT> &print, int i); 
  void SetInferredWall(const std::vector<WALL_FINGERPRINT> &print, int i);
  void SetTinyUnusedWall(const std::vector<WALL_FINGERPRINT> &print, int i);
  void LabelAdditionalInteriors();
  void LabelInferredWalls();  
  void LabelTinyUnusedWalls();  
  void SearchForInteriorInferredWalls();

  void ConnectedGroupAnalysis() {
    for (std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData>::iterator iter=groups.begin(); iter!=groups.end(); iter++) {
      iter->second.ConnectedGroupAnalysis();
    }
  }
  // print
  void Print() const;
  void PrintStats() const;
  void InsertPolygon(Poly *p);

  void TabulateEnclosure(ArgParser *args,double tuned_enclosure_threshold);
  void PickInteriorZones(ArgParser *args, std::vector<ZoneData> &zones, double tuned_enclosure_threshold);
  void WallAnalysis(ArgParser *args, std::vector<WallEvidence> &all_wall_evidence);
  bool SplitMixedEnclosureGroups(double tuned_enclosure_threshold);
private:

  int LabelInferredWallsHelper();  
  //  int LabelInteriorInferredWallsHelper();  

  // representation
  std::map<std::vector<WALL_FINGERPRINT>,PolygonGroupData> groups;
};

// ===============================================================================================
// ===============================================================================================

void printPrint(const std::vector<WALL_FINGERPRINT> &p1);
std::ostream& operator<<(std::ostream& ostr, const std::vector<WALL_FINGERPRINT> &p1);
bool noMiddle(const std::vector<WALL_FINGERPRINT> &p1);
Vec3f FingerprintColor(ArgParser *args, const std::vector<WALL_FINGERPRINT> &print,int which_subgroup);
Vec3f NumMiddlesColor(const std::vector<WALL_FINGERPRINT> &print);
int NumMiddles(const std::vector<WALL_FINGERPRINT> &print);



inline bool isWallOrWindow(enum ARRANGEMENT_CELL_TYPE type) {
  return (type == ACT_WALL || 
	  type == ACT_WINDOW_CYAN || type == ACT_WINDOW_MAGENTA || type == ACT_WINDOW_YELLOW);
}



#endif
