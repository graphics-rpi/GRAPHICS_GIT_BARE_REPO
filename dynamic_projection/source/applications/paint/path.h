#ifndef _PATH_H_
#define _PATH_H_

#include "gl_includes.h"

#include <deque>
#include "ClickableObject.h"
#include "../../../../remesher/src/vectors.h"

class Path : public virtual ClickableObject{
    public:
        Path(const Vec3f &c, int w) : ClickableObject(Pt(0,0), w, w, c ){
            color = c;
            path_width = w;
        }
        Path(const std::deque<Pt> &t, const Vec3f &c, int w) : ClickableObject(Pt(0,0), w, w, c) {
            trail = t;
            color = c;
            path_width = w;
            magnified_trail = t;
        }
        void draw() const;

        static void draw_smooth_stroke(const std::deque<Pt> &trail, double width=10) {

            draw_smooth_stroke2(trail, width);
            //draw_polygonal_stroke2(trail);
            //draw_variable_line(trail);
            //draw_variable_quad(trail);
        }

        virtual void populatePath(std::vector<glm::vec3> & edge_positions, 
                std::vector<glm::vec4> & edge_colors, 
                std::vector<GLfloat> & edge_widths);
        virtual void populateJoints(std::vector<glm::vec3> & vertex_positions,
                std::vector<glm::vec4> & vertex_colors,
                std::vector<GLfloat> & vertex_radii);
        virtual bool displayText(){ return false; }
        virtual std::string getText(){ return std::string(""); }

        void setMagnifiedTrail(std::deque<Pt> trail){ magnified_trail = trail; }
        void setOriginalTrail(std::deque<Pt> trail){ this->trail = trail; }

        std::deque<Pt> getOriginalTrail(){ return trail; }
        std::deque<Pt> getMagnifiedTrail(){ return magnified_trail; }

        // Allow paths to be created...
        virtual double DistanceFrom(const Pt &/*p*/){ return 0.0; }
        virtual void paint(const Vec3f & /*background_color = Vec3f(0,0,0)*/) const { draw(); }
        virtual bool isVisible() const { return true; }
        virtual double getZOrder() const { return 0.0; }

    private:

        static void draw_polygonal_stroke2(const std::list<Pt> &trail);
        static void draw_smooth_stroke2(const std::deque<Pt> &trail, double width);
        static void draw_trail_line(const std::list<Pt> &upper, const std::list<Pt> &lower);
        static void draw_trail_quad(const std::list<Pt> &upper, const std::list<Pt> &lower);
        static void generate_trail(const std::deque<Pt> &trail, double radius, std::list<Pt> &lower, std::list<Pt> &upper);

        static void smooth_trail(std::list<Pt> &trail);
        static void generate_bspline(const std::deque<Pt> &trail, std::deque<Pt> &result);


    protected:
        std::deque<Pt> trail;
        std::deque<Pt> magnified_trail;
        int path_width;

};


#endif
