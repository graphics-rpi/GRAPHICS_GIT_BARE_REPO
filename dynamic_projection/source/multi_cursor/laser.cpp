#include "laser.h"


void Laser::draw(bool screen_space, const Vec3f &background_color) const {

/*
  assert (global_point_tracker != NULL);

  for (unsigned int i = 0; i < global_point_tracker->size(); i++) {
    int id = (*global_point_tracker)[i].getID();
    bool pressing = false;
    // check if this tracker is attached to a button
    for (unsigned int j = 0; j < GLOBAL_buttons.size(); j++) {
      if (GLOBAL_buttons[j]->isPressed() && GLOBAL_buttons[j]->PressedBy() == id)
	pressing = true;
    }
    if (!pressing && button_strokes) continue;
    if (pressing && !button_strokes) continue;

    if (!(*global_point_tracker)[i].IsActive()) continue;

    // SET COLOR
    Vec3f color = global_colors.GetColor(id);
    glColor3f(color.x(),color.y(),color.z());

    // DRAW TRAIL
    glLineWidth(3);
    Path::draw_smooth_stroke((*global_point_tracker)[i].getPtTrail());
  }
*/

}
