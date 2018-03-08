#include "QTElement.h"
#include "QTNode.h"
#include "QTEdge.h"

QTElement::QTElement( unsigned int id, const Pt &p, double w, double h, const Vec3f &c /*const Pt & _a, const Pt & _b,  bool reorder*/ )
  : ClickableObject(p,w,h,c)//,
    //boundingbox( p,p) //,false) //_a, _b, reorder)
{
  my_id = id;  
}


/*void QTElement::setBox(const Pt & a, const Pt & b){
  // THIS SHOULD NOT BE USED UNLESS ABSOLUTELY NECESSARY.
  // IT MESSES UP THE QTREE, IF YOU CHANGE AN ELEMENT, YOU
  // MUST REMOVE AND READD IT.
  boundingbox.SetBox(a, b);
}
*/

const BoundingBox2f& QTElement::getBox() const{
    return boundingbox;
}


std::ostream& operator<< (std::ostream &ostr, const QTElement &elem) {  
  const QTNode *node = dynamic_cast<const QTNode*>(&elem);
  if (node != NULL) { ostr << *node; }
  else { 
    const QTEdge *edge = dynamic_cast<const QTEdge*>(&elem);
    if (edge != NULL) { ostr << *edge; }
    else { assert(0); }
  }
  return ostr;
}
  

