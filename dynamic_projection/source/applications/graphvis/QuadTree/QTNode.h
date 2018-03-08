/*
 * QTNode.h
 *
 *  Created on: Jun 17, 2011
 *      Author: phipps
 */


extern double SELECTING_COUNTER_HACK;

#ifndef QTNODE_H_
#define QTNODE_H_
#include "QTEdge.h"
#include "Metadata.h"
#include "QTElement.h"
#include "../paint/button.h"
#include "../paint/BoundingBox2f.h"
#include "../paint/DrawableObject.h"


class QTEdge;
class Layer;


// ==========================================================================
// ==========================================================================
class QTNode : public Button, public QTElement {
    friend std::ostream& operator<< (std::ostream &ostr, const QTNode &e);

    public:
        // CONSTRUCTORS, ETC.
        QTNode(int id, Layer *layer,
                const std::string &texname,const Pt &p_, const Vec3f &_color,
                Metadata t, double relativesize);

        // ACCESSORS
        const Metadata *GetData() const { return &data; }
        unsigned int NumConnections() const {return connections.size();}
        QTEdge *GetConnection(int _ID) const {return connections[_ID];}
        Pt getPosition() {return getCentroid(); }
        double GetConnectionDistance(QTNode *n) const;
        int GetDrawMode() const {return draw_mode;}

        virtual bool isVisible() const;
        virtual double  getZOrder() const;

        const Vec3f& getColor() const { return color; }
        bool IsConnected(QTNode *n) const;
        const std::string& getType() const;
        Layer* getLayer() const { return layer; }


        // MODIFIERS
        Metadata *GetData() { return &data; }
        void setPosition(double x, double y);
        void SetDrawMode(int i);

        void AddConnection(QTEdge *e);
        void RemoveConnection(QTEdge *e);

        void Draw() const;
        virtual void paint(const Vec3f &background_color=Vec3f(0,0,0)) const { Draw(); }
        bool AnimateSize();

        void updateButtonText();

        bool checkStatus(long field);

        //Hovering Helpers
        float GetCounter(){return counter;}
        void SetCounter(float _counter);

        float select_recover;
        void setButtonBorders();

        bool visited;


    protected:

    private:
        std::vector<QTEdge*> connections;

        Layer* layer;
        Metadata data;
        double relativesize;

        double raw_size;
        double current_scale;

        //This mode is related to the animatesize function
        //draw_mode = 4 -> node is open and expanded on the graph
        int draw_mode;

        float counter;

        double power_percentage;
        double water_percentage;
        double waste_percentage;
        double comm_percentage;
};

#endif /* QTNODE_H_ */
