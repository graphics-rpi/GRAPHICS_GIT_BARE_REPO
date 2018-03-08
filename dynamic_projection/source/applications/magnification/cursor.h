#ifndef __CURSOR_H__
#define __CURSOR_H__

#include "../../common/glm/glm/glm.hpp"
#include "../../common/glm/glm/gtx/string_cast.hpp"
#include "../../common/glm/glm/gtc/type_ptr.hpp"


#include "canvas.h"
#include "argparser.h"
#include <random> 

// note to self, mesh set next pos w/ GenerateNewDest

class Cursor{
    public:
        // Constructor
        Cursor( ArgParser* args, int id, int x, int y); 

        // Drawing
        static void Draw();
        
        // Updating
        virtual void Update();

        virtual void Print( std::ostream & stream = std::cout ) const;
        
        // DONT DO THIS, BAD PRACTICIES FOR NOW, FIGURE SOLUTION LATER.
        virtual void set_next_pos( const glm::vec2 & next_pos ) { return; }
        // Helper functions
        glm::vec2 ConvertToModelCoordinates(const glm::vec2 & input);

        glm::vec2 pos() const { return pos_; }

    protected:
        int id_;
        glm::vec2 pos_;
        glm::vec3 color_;

        std::mt19937 rng_;
        static ArgParser* args_;

        static GLuint cursor_pos_vbo_;
        static GLuint cursor_color_vbo_;
        static GLuint cursor_vao_;
        static bool initialized_; 

        static glm::mat4 model_;
};

class AICursor : public Cursor {
    public:
        AICursor(ArgParser* args, int id, int x, int y);

        void Update();

        virtual void Print( std::ostream & stream = std::cout ) const;
    private:
        void GenerateNewDest();
        glm::vec2 vel_;
        glm::vec2 dest_;

        std::uniform_int_distribution<> x_dis_;
        std::uniform_int_distribution<> y_dis_;
};

class ControlledCursor : public Cursor{
    public:
        ControlledCursor(ArgParser* args, int id, int x, int y) : Cursor( args, id, x, y), next_pos_(x,y){}
        void Update();
        virtual void Print( std::ostream & stream = std::cout ) const;
        
        void set_next_pos( const glm::vec2 & new_pos ){ next_pos_ = new_pos; }
    private:
        glm::vec2 next_pos_;
};

inline std::ostream& operator<<(std::ostream& stream, const Cursor & cursor){
    cursor.Print(stream);
    return stream;
}

#endif
