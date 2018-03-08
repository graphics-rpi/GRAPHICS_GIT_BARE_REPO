#ifndef DRAWABLEOBJECT_H
#define DRAWABLEOBJECT_H

#include <iostream>

#include "BoundingBox2f.h"

class BorderInfo {
public:
    BorderInfo(const Vec3f &c, double t, double o) : color(c), thickness(t), opacity(o) {}
    Vec3f color;
    double thickness;
    double opacity;
};


// Abstract base class
class DrawableObject{

public:

    // ========================
    // DIMENSIONS & POSITION
    void SetScaleFactor(double sf) { scale_factor = sf; }
    double getWidth() const { return raw_width*scale_factor; }
    double getHeight() const { return raw_height*scale_factor; }
    double getScaleFactor() const { return scale_factor; }
    Pt getLowerLeftCorner() const { return computed_centroid - Pt(getWidth()/2.0,getHeight()/2.0); }
    Pt getUpperRightCorner() const { return computed_centroid + Pt(getWidth()/2.0,getHeight()/2.0); }
    const Pt& getCentroid() const { return computed_centroid; }
    void setDimensions(double w, double h) {
      assert (w > 0 && h > 0);
      raw_width = w; raw_height = h;
    }

    Pt getMagnifiedLowerLeftCorner() const { return magnified_centroid - Pt(getWidth()/2.0,getHeight()/2.0); }
    Pt getMagnifiedUpperRightCorner() const { return magnified_centroid + Pt(getWidth()/2.0,getHeight()/2.0); }
    const Pt& getMagnifiedCentroid() const { return magnified_centroid; }

    void setMagnifiedCentroid(double x, double y){ magnified_centroid = Pt(x,y); };

    virtual bool PointInside(const Pt &p) const;
    virtual double DistanceFrom(const Pt &p) const = 0;

    Pt Offset(const Pt &p) const;
    void Move(const Pt &p);
    void MoveChooseDampingCoeff(const Pt &p, double dampingCoeff);
    void MoveNoDamping(const Pt &p);
    void MoveNoDampingCentroid(const Pt &p);
    void MoveWithMaxSpeed(const Pt &p, double max_speed);

    // ==========================
    // RENDERING
  virtual void paint(const Vec3f &background_color=Vec3f(0,0,0)) const =0;
    void setTransparency(double t) { transparency = t; }
    void setColor(const Vec3f &c) { color = c; }
    Vec3f getColor() const { return color; }
    void addBorder(const BorderInfo &bi) {
        border_info.push_back(bi);
    }
    bool hasBorder() { return border_info.size() != 0; }
    Vec3f getSingleBorderColor() {
        assert (border_info.size() == 1);
        return border_info[0].color;
    }
    void clearBorders() { border_info.clear(); }

    virtual bool isVisible() const =0;
    virtual double  getZOrder() const =0;



protected:
    // ==========================
    // CONSTRUCTORS
  DrawableObject(const Pt &lower_left_corner, double w, double h, const Vec3f &c);
  DrawableObject();

    // ==========================
    // HELPER FUNCTIONS
  void initialize(const Pt &lower_left_corner, double w, double h, const Vec3f &c);

    // ==========================
    // REPRESENTATION

    // geometry
    Pt computed_centroid; 
    Pt magnified_centroid;
    double scale_factor;
    double raw_width;
    double raw_height;

    // appearance
    Vec3f color;
    std::vector<BorderInfo> border_info;
    double transparency;
};

#endif
