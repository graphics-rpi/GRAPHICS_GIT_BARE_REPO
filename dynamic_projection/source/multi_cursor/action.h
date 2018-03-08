#ifndef _ACTION_H_
#define _ACTION_H_

enum action_type { ACTION_DRAG, ACTION_EXPAND, ACTION_COLLAPSE };

class Action {
 public:
  static Action Hover(double r, Pt &cent, unsigned long tpi) {
    Action answer;
    answer.radius = r;
    answer.center = cent;
    //std::cout<< " hover " << cent.x << " " << cent.y << std::endl;
    answer.color = Vec3f(0,1,0);
    answer.type = ACTION_DRAG;
    answer.action_count = 1;
    answer.tracked_point_id = tpi;
    return answer;
  }
  static Action Circle(double r, Pt &cent, unsigned long tpi) {
    Action answer;
    answer.radius = r;
    answer.center = cent;
    answer.color = Vec3f(0,0,1);
    answer.type = ACTION_EXPAND;
    answer.action_count = 1;
    answer.tracked_point_id = tpi;
    return answer;
  }
  static Action ZStrike(double r, Pt &cent, unsigned long tpi) {
    Action answer;
    answer.radius = r;
    answer.center = cent;
    answer.color = Vec3f(1,0,0);
    answer.type = ACTION_COLLAPSE;
    answer.action_count = 1;
    answer.tracked_point_id = tpi;
    return answer;
  }


  double getRadius() const { return radius; }
  const Pt& getCenter() { return center; }
  const Vec3f& getColor() const { return color; }
  bool is_drag() const { return (type == ACTION_DRAG); }
  bool is_collapse() const { return (type == ACTION_COLLAPSE); }
  bool is_expand() const { return (type == ACTION_EXPAND); }
  bool same_action_type(const Action &a) const { return (type == a.type); }
  
  void advanceActionCount(const Action &a) {
    assert (action_count == 1);
    action_count = a.action_count+1;
  }
  int getActionCount() const {
    return action_count;
  }

  unsigned long getTrackedPointID() const { return tracked_point_id; }

private:
  double radius;
  Pt center;
  Vec3f color;
  action_type type;
  int action_count;

  unsigned long tracked_point_id;
};

#endif