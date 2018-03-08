#include "../paint/gl_includes.h"

#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "button.h"
#include "ClickableObject.h"
#include "text.h"
#include "../../common/Image.h"
#include "../../multi_cursor/interaction.h"

int FLIP_ALL_TEXT = false;

#define CIRCLE_RESOLUTION 20

//extern int HandleGLError(std::string foo);

// HACK button border was this big for puzzle?????
//#define BUTTON_BORDER 25
//#define BUTTON_BORDER 5
#define DAMP_MOVEMENT 0.2


void Button::initialize() {
    // appearance
    render_text = false;
    is_circle = false;
    bold_text = false;

    // texture
    texture_enabled = false;
    sm_texName = 0; // no texture  
    lg_texName = 0; // no texture  
    sm_image = NULL;
    lg_image = NULL;
    u0 = v0 = 0.0;
    u1 = v1 = 1.0;
}

Button::Button(const Pt &p, double w, double h, const Vec3f &c, const std::string &t) : ClickableObject(p,w,h,c) {
    assert (w > 0 && h > 0);
    initialize();
    addText(t);
}

Button::Button(const Pt &p, double w, double h, const Vec3f &c, 
        const std::string &small_texture_filename, const std::string &large_texture_filename, 
        double u0_, double v0_, double u1_, double v1_) : ClickableObject(p,w,h,c) {
    assert (w > 0 && h > 0);
    initialize();
    specify_texture(small_texture_filename,large_texture_filename,u0_,v0_,u1_,v1_);
}

void Button::specify_texture(const std::string &small_texture_filename, const std::string &large_texture_filename,
        double u0_, double v0_, double u1_, double v1_) {
    u0 = u0_;
    v0 = v0_;
    u1 = u1_;
    v1 = v1_;
    sm_image_filename = small_texture_filename;
    lg_image_filename = large_texture_filename;
    assert (sm_texName == 0);
    assert (lg_texName == 0);
    assert (sm_image == NULL);
    assert (lg_image == NULL);
}

void Button::specify_uv_coordinates(double u0, double v0, double u1, double v1){
    this->u0 = u0;
    this->v0 = v0;
    this->u1 = u1;
    this->v1 = v1;
}

Button::~Button() {

    /*
    // can't delete for sure, they might be shared :(

    delete image_small;
    delete image_large;
    */

}


void Button::addText(const std::string& t) { 
    std::string foo;
    for (unsigned int i = 0; i < t.size(); i++) {
        if (t[i] == '\n') {
            if (foo != "") text_strings.push_back(foo);
            foo = "";
        } else {
            foo += t[i];
        }
    }
    if (foo != "") {
        text_strings.push_back(foo);
        render_text = true;
    }
}


void Button::load_small_texture() {
    assert (sm_image_filename != "");
    assert (sm_image == NULL);
    assert (sm_texName == 0);
    sm_image = loadImage(sm_image_filename, &sm_texName);
}

void Button::load_large_texture() {
    assert (lg_image_filename != "");
    assert (lg_image == NULL);
    assert (lg_texName == 0);
    lg_image = loadImage(lg_image_filename, &lg_texName);
}

// These two functions load the image if not already loaded
bool Button::SmallImageSpecifiedAndLoaded() const {
    if (sm_image_filename == "") return false;
    if (sm_image != NULL) {
        assert (sm_texName != 0);
        return true;
    } else {
        assert (sm_texName == 0);
        ((Button*)this)->load_small_texture();
        return true;
    }
}

bool Button::LargeImageSpecifiedAndLoaded() const {
    if (lg_image_filename == "") return false;
    if (lg_image != NULL) {
        assert (lg_texName != 0);
        return true;
    } else {
        assert (lg_texName == 0);
        ((Button*)this)->load_large_texture();
        return true;
    }
}

bool Button::isVisible() const {
    return true;
}

double Button::getZOrder() const {
    return 0;
}

// =================================================================================
// =================================================================================

void draw_circle(const Pt &pt, double radius, int num) {
    assert (radius > 0);
    assert (num > 3);
    glBegin(GL_POLYGON);
    for (int i = 0; i <= num; i++) {
        double angle = i*2*M_PI / double(num);
        Pt other = pt + radius * Pt(cos(angle),sin(angle));
        glVertex2f(other.x,other.y);
    }
    glEnd();
}

void draw_circle_with_texture(const Pt &pt, double radius, int num) {
    assert (radius > 0);
    assert (num > 3);
    glBegin(GL_POLYGON);
    for (int i = 0; i <= num; i++) {
        double angle = i*2*M_PI / double(num);
        Pt other = pt + radius * Pt(cos(angle),sin(angle));
        Pt textcoord = Pt(0.5,0.5)+0.5*Pt(cos(angle),sin(angle));
        glTexCoord2f(textcoord.x,1-textcoord.y);
        glVertex2f(other.x,other.y);
    }
    glEnd();
}

void Button::populateRectangleBorder(std::vector<glm::vec3> & positions, std::vector<glm::vec4> & colors){
    double total_border = 0.1;
    double z = getZOrder();
    for (unsigned int i = 0; i < border_info.size(); i++) {
        total_border += border_info[i].thickness*scale_factor;
    }

    // TODO: Figure out way to only pass in one color per triangle
    for (int i = (int)border_info.size()-1; i >= 0; i--) {
        Vec3f border_color = border_info[i].color;
        double thickness = total_border;
        double border_opacity = border_info[i].opacity;

        glm::vec4 color(border_color.r(), border_color.g(), border_color.b(), border_opacity);

        Pt pt = getMagnifiedLowerLeftCorner();
        positions.push_back(glm::vec3((float)pt.x-thickness,(float)pt.y-thickness, z));
        colors.push_back(color);
        positions.push_back(glm::vec3((float)pt.x+getWidth()+thickness,(float)pt.y-thickness, z));
        colors.push_back(color);
        positions.push_back(glm::vec3((float)pt.x+getWidth()+thickness,(float)pt.y+getHeight()+thickness, z));
        colors.push_back(color);

        positions.push_back(glm::vec3((float)pt.x-thickness,(float)pt.y-thickness, z));
        colors.push_back(color);
        positions.push_back(glm::vec3((float)pt.x+getWidth()+thickness,(float)pt.y+getHeight()+thickness, z));
        colors.push_back(color);
        positions.push_back(glm::vec3((float)pt.x-thickness,(float)pt.y+getHeight()+thickness, z));
        colors.push_back(color);

        total_border -= border_info[i].thickness*scale_factor;
    }

}

void Button::populateRectangleButton(std::vector<glm::vec3> & positions, std::vector<glm::vec4> & colors){
    double offset = getHeight() * 0.2;
    double z = getZOrder();

    Vec3f tmp = color;
    glm::vec4 first_color(tmp.r(), tmp.g(), tmp.b(), 0.5f);


    Pt pt = getMagnifiedLowerLeftCorner();
//    printf("%d, %d\n", pt.x, pt.y);
    positions.push_back(glm::vec3((float)pt.x,(float)pt.y, z));
    colors.push_back(first_color);
    positions.push_back(glm::vec3((float)pt.x+getWidth(),(float)pt.y, z));
    colors.push_back(first_color);
    positions.push_back(glm::vec3((float)pt.x+getWidth(),(float)pt.y+getHeight(), z));
    colors.push_back(first_color);

    positions.push_back(glm::vec3((float)pt.x,(float)pt.y, z));
    colors.push_back(first_color);
    positions.push_back(glm::vec3((float)pt.x+getWidth(),(float)pt.y+getHeight(), z));
    colors.push_back(first_color);
    positions.push_back(glm::vec3((float)pt.x,(float)pt.y+getHeight(), z));
    colors.push_back(first_color);

    if (transparency > 0.01) {
        glm::vec4 second_color(tmp.r(), tmp.g(), tmp.b(), 0.8f);

        positions.push_back(glm::vec3((float)pt.x+offset      ,(float)pt.y+offset, z));
        colors.push_back(second_color);
        positions.push_back(glm::vec3((float)pt.x+getWidth()-offset,(float)pt.y+offset, z));
        colors.push_back(second_color);
        positions.push_back(glm::vec3((float)pt.x+getWidth()-offset,(float)pt.y+getHeight()-offset, z));
        colors.push_back(second_color);

        positions.push_back(glm::vec3((float)pt.x+offset      ,(float)pt.y+offset, z));
        colors.push_back(second_color);
        positions.push_back(glm::vec3((float)pt.x+getWidth()-offset,(float)pt.y+getHeight()-offset, z));
        colors.push_back(second_color);
        positions.push_back(glm::vec3((float)pt.x+offset      ,(float)pt.y+getHeight()-offset, z));
        colors.push_back(second_color);
    }
}

void Button::populateRectangleTexturedButton(std::vector<glm::vec3> & positions, std::vector<glm::vec2> & uvs){
    Pt pt = getMagnifiedLowerLeftCorner();
    double z = getZOrder();

    //std::cout << pt.x << " " << pt.y << std::endl;

    positions.push_back(glm::vec3((float)pt.x,(float)pt.y, z));
    uvs.push_back(glm::vec2((float)u0, (float)v1));
    positions.push_back(glm::vec3((float)pt.x+getWidth(),(float)pt.y, z));
    uvs.push_back(glm::vec2((float)u1, (float)v1));
    positions.push_back(glm::vec3((float)pt.x+getWidth(),(float)pt.y+getHeight(), z));
    uvs.push_back(glm::vec2((float)u1, (float)v0));

    positions.push_back(glm::vec3((float)pt.x,(float)pt.y, z));
    uvs.push_back(glm::vec2((float)u0, (float)v1));
    positions.push_back(glm::vec3((float)pt.x+getWidth(),(float)pt.y+getHeight(), z));
    uvs.push_back(glm::vec2((float)u1, (float)v0));
    positions.push_back(glm::vec3((float)pt.x,(float)pt.y+getHeight(), z));
    uvs.push_back(glm::vec2((float)u0, (float)v0));

}

void Button::populateCircleBorder(std::vector<glm::vec3> & positions,
        std::vector<glm::vec4> & colors,
        std::vector<GLfloat> & radii){
    double z = getZOrder();

    // Calculate total size of border
    double total_border = 0.1;
    for (unsigned int i = 0; i < border_info.size(); i++) {
        total_border += border_info[i].thickness * scale_factor;
    }

    for( int i = (int)border_info.size()-1; i >= 0; i-- ){

        Vec3f border_color = border_info[i].color;
        double thickness = total_border;
        double border_opacity = border_info[i].opacity;
        Pt center = getMagnifiedCentroid();

        glm::vec3 position( (float)center.x, (float)center.y, z );
        glm::vec4 color(border_color.r(), border_color.g(), border_color.b(), border_opacity);
        float radius = (getHeight() + thickness) * 0.5f;

        // Insert into array
        positions.push_back(position); 
        colors.push_back(color);
        radii.push_back(radius);
       
        total_border -= border_info[i].thickness * scale_factor;
    }

    return;
}
void Button::populateCircleButton(std::vector<glm::vec3> & positions,
        std::vector<glm::vec4> & colors,
        std::vector<GLfloat> & radii){
    double z = getZOrder();

    Pt center = getMagnifiedCentroid();

    positions.push_back(glm::vec3((float)center.x, (float)center.y, z ));
    colors.push_back(glm::vec4(color.r(), color.g(), color.b(), 1-transparency));
    radii.push_back(getHeight() * 0.5f);

    return;
}
void Button::populateCircleTexturedButton(std::vector<glm::vec3> & positions,
        std::vector<glm::vec2> & uvs,
        std::vector<GLfloat> & radii){
    double z = getZOrder();

    Pt center = getMagnifiedCentroid();

    positions.push_back(glm::vec3((float)center.x, (float)center.y, z ));
    uvs.push_back(glm::vec2((u1 - u0) / 2.0 + u0 , (v1 - v0) / 2.0 + v0));
    radii.push_back(getHeight() * 0.5f);

    return;
}

void Button::paint(const Vec3f &background_color)  const
{
    HandleGLError("gl error at BUTTON PAINT START");

    std::cout << "SKIPPING BUTTON PAINT" << std::endl;

    /*
    // ===========================================
    // RENDER THE BORDER
    double total_border = 0.1;
    for (unsigned int i = 0; i < border_info.size(); i++) {
        total_border += border_info[i].thickness*scale_factor;
    }

    for (int i = (int)border_info.size()-1; i >= 0; i--) {
        Vec3f border_color = border_info[i].color;
        double thickness = total_border;
        total_border -= border_info[i].thickness*scale_factor;
        double border_opacity = border_info[i].opacity;
        glColor4f(border_color.r(),border_color.g(),border_color.b(), border_opacity);
        glEnable(GL_BLEND);
        if (is_circle && !render_text) {
            draw_circle(getCentroid(),(getHeight()+thickness)*0.5,CIRCLE_RESOLUTION);
        } else {
            glBegin(GL_QUADS);
            Pt pt = getLowerLeftCorner();
            glVertex2f(pt.x-thickness,pt.y-thickness);
            glVertex2f(pt.x+getWidth()+thickness,pt.y-thickness);
            glVertex2f(pt.x+getWidth()+thickness,pt.y+getHeight()+thickness);
            glVertex2f(pt.x-thickness,pt.y+getHeight()+thickness);
            glEnd();
        } 
        glDisable(GL_BLEND);
    }

    HandleGLError("done with border");

    // ===========================================
    // RENDER THE BUTTON

    if (render_text == false && is_circle) { 
        // SMALL!
        if (!SmallImageSpecifiedAndLoaded()) {
          //glColor4f(color.r(),color.g(),color.b(),1-transparency);
            draw_circle(getCentroid(),getHeight()*0.5,CIRCLE_RESOLUTION);
        } else {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, getSmallTexName());
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
            draw_circle_with_texture(getCentroid(),getHeight()*0.5,CIRCLE_RESOLUTION);
            glDisable(GL_TEXTURE_2D);
        }
    } else {
        // LARGE!
        if (!is_texture_enabled() || !LargeImageSpecifiedAndLoaded()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            double offset = getHeight() * 0.2;
            glBegin(GL_QUADS);

            Vec3f tmp = background_color;
            //glColor4f(tmp.r(),tmp.g(),tmp.b(),0.5); //1-transparency);

            //glColor4f(color.r(),color.g(),color.b(),1-transparency);
            Pt pt = getLowerLeftCorner();
            glVertex2f(pt.x,pt.y);
            glVertex2f(pt.x+getWidth(),pt.y);
            glVertex2f(pt.x+getWidth(),pt.y+getHeight());
            glVertex2f(pt.x,pt.y+getHeight());
            if (transparency > 0.01) {
              //glColor4f(tmp.r(),tmp.g(),tmp.b(),0.8); //-transparency);

                //Vec3f tmp = color*(1-transparency) + background_color*transparency;
                //glColor4f(tmp.r(),tmp.g(),tmp.b(),1);
                glVertex2f(pt.x+offset      ,pt.y+offset);
                glVertex2f(pt.x+getWidth()-offset,pt.y+offset);
                glVertex2f(pt.x+getWidth()-offset,pt.y+getHeight()-offset);
                glVertex2f(pt.x+offset      ,pt.y+getHeight()-offset);
            }
            glEnd();
            glDisable(GL_BLEND);
        } else {
          //glColor3f(color.r(),color.g(),color.b());
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, getLargeTexName());
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
            glBegin(GL_QUADS);
            Pt pt = getLowerLeftCorner();
            glTexCoord2f(u0,v0);        glVertex2f(pt.x,pt.y+getHeight());
            glTexCoord2f(u0,v1);        glVertex2f(pt.x,pt.y);
            glTexCoord2f(u1,v1);        glVertex2f(pt.x+getWidth(),pt.y);
            glTexCoord2f(u1,v0);        glVertex2f(pt.x+getWidth(),pt.y+getHeight());
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
    }

    HandleGLError("done with button");

    // ===========================================
    // RENDER THE TEXT
    int num_strings = text_strings.size();    
    if(num_strings > 0 && render_text) {

        Pt pt = getLowerLeftCorner();
        // COLOR
        Vec3f textcolor;
        if (color.r() > 0.9 && color.g() > 0.75) {
            // white or yellow
            textcolor = 0.5*color;
        } else {
            // dark colors
            textcolor = 0.75*Vec3f(1,1,1) + 0.25*color;
        }

        if (is_texture_enabled()) {
            textcolor = Vec3f(1,1,1);
        }

        double x = pt.x+0.5*getWidth();
        double lineheight = getHeight() / (double)num_strings;

        //Vec3f color = textcolor;

        if (!isBoldText()) {
            for(int i = 0; i < num_strings; ++i) {
                double y = pt.y + lineheight * ((num_strings-i)-0.5);
                drawstring(x,y,
                        text_strings[i].c_str(),
                        textcolor,
                        0.9*getWidth(),
                        0.9*getHeight() / (double)text_strings.size());
            }
        } else {

            textcolor = Vec3f(1,1,1) - background_color;

            if (is_texture_enabled()) {
                textcolor = Vec3f(1,1,1);
            }

            for(int i = 0; i < num_strings; ++i) {
                double y = pt.y + lineheight * ((num_strings-i)-0.5);
                drawstring(x,y,
                        text_strings[i].c_str(),
                        background_color * 0.7 + textcolor*0.3, //;Vec3f(0.3,0.3,0.3),
                        0.9*getWidth(),
                        0.9*getHeight() / (double)text_strings.size());
                drawstring(x,y,
                        text_strings[i].c_str(),
                        textcolor, //Vec3f(1,1,1),
                        0.9*getWidth(),
                        0.9*getHeight() / (double)text_strings.size());
            }
        }

        //    return;
    }
    */

    HandleGLError("done with text");
}

// =================================================================================
// =================================================================================

bool Button::PointInside(const Pt &p) const {  
    Pt pt = getLowerLeftCorner();
    //  std::cout << "PointInside " << p.x << " " << p.y << "    " << pt.x << " " << pt.y << "    " << pt.x+getWidth() << " " << pt.y+getHeight() << std::endl;
    bool answer =  (p.x > pt.x && 
            p.y > pt.y && 
            p.x <= pt.x+getWidth() && 
            p.y <= pt.y+getHeight());
    if (answer) {
        // std::cout << "INSIDE!" << std::endl;
    }
    else {
        //    std::cout << "not inside" << std::endl;
    }
    return answer;
}

double Button::DistanceFrom(const Pt &p) const {  
    Pt pt = getLowerLeftCorner();
    if (PointInside(p)) return 0;
    double x_dist,y_dist;
    if (p.x < pt.x) {
        x_dist = pt.x-p.x;
    } else if (p.x > pt.x+getWidth()) {
        x_dist = p.x - pt.x+getWidth();
    } else {
        x_dist = 0;
    }
    if (p.y < pt.y) {
        y_dist = pt.y-p.y;
    } else if (p.y > pt.y+getHeight()) {
        y_dist = p.y - pt.y+getHeight();
    } else {
        y_dist = 0;
    }
    double answer = sqrt(x_dist*x_dist+y_dist*y_dist);
    return answer;
}

Image<sRGB>* Button::loadImage(const std::string &filename, GLuint *tn) {

  HandleGLError("enter Button::loadImage()");

    // =================================================================
    // a static map to help us when we load lots of the same image!
    static std::map<std::string,std::pair<GLuint,Image<sRGB>*> > loaded;
    // =================================================================


    std::map<std::string,std::pair<GLuint,Image<sRGB>*> >::iterator itr = loaded.find(filename);

    if (itr != loaded.end()) {
        *tn = itr->second.first;
        return itr->second.second;
    }

    std::cout << "load texture image "<< filename << std::endl;

    Image<sRGB>* answer = new Image<sRGB>;
    try {
        answer->load(filename); 
    } catch (...) {
        // something went wrong
        delete answer;
        tn = 0;
        return NULL;
    }

  HandleGLError("in Button::loadImage() A");

    int w = answer->getCols();
    int h = answer->getRows();
    GLubyte *gl_image = new GLubyte[w*h*3];

    for (int i=0; i<h; i++){
        for (int j=0; j<w; j++){
            sRGB c = (*answer)(i,j);
            GLubyte r = c.r();
            GLubyte g = c.g();
            GLubyte b = c.b();
            gl_image[i*w*3 + j*3 + 0] = r;
            gl_image[i*w*3 + j*3 + 1] = g;
            gl_image[i*w*3 + j*3 + 2] = b;
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, tn); 

    loaded[filename] = std::make_pair(*tn,answer);

  HandleGLError("in Button::loadImage() C");

    glBindTexture(GL_TEXTURE_2D, *tn); //exName);
    //    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);


    // FIXME HACK TODO:  IF THIS IS NEEDED --- CHECK EXACT SYNTAX IS CAUSING GL ERROR ON MAC
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  HandleGLError("in Button::loadImage() D");

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, w, h, GL_RGB, GL_UNSIGNED_BYTE, gl_image);

    //  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w,h, 
    //         0, GL_RGB, GL_UNSIGNED_BYTE, 
    //         gl_image);


    delete [] gl_image;

    
  HandleGLError("leaving Button::loadImage()");

    return answer;
}
