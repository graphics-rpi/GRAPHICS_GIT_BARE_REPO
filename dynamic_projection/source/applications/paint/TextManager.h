#ifndef TEXT_MANAGER_H_
#define TEXT_MANAGER_H_

#include "gl_includes.h"

#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "base_argparser.h"

namespace TextStyles{
    enum Enum {
        REGULAR,
        BOLD,
        ITALIC
    };
}

namespace TextVBO{
    enum Enum {
        POSITION,
        UV,
        COLOR
    };
}

struct CharInfo{
    float ax;
    float ay;

    float bw;
    float bh;

    float bl;
    float bt;

    float tx;
};

class Font{
    public:
        Font(FT_Library ft, std::string font_name, int size, TextStyles::Enum style);

        ~Font();

        void createFontTexture();
        void bindFont();

        CharInfo getCharInfo( int index ){ return m_characters[index]; }

        float getWidth(){ return (float)m_width; }
        float getHeight(){ return (float)m_height; }
        

    private:
        void setFontTextureDimensions();
        void createSignedDistanceField(int height, int width, unsigned char* buffer, unsigned char* result);
        int closestDistance( int x, int y, int color, int state[], int height, int width );

        std::string m_name;
        FT_Face m_face;

        int m_width;
        int m_height;

        GLuint m_tex;
        
        CharInfo m_characters[128];
};

class TextManager{
    public:
        TextManager(ArgParser* args);
        TextManager(std::string font);

        //void addString(const char* text, float x, float y);
        void addString(const char* text, float x, float y, float z, float sx, float sy, glm::vec3 text_color,
                glm::vec2 max_dimension);
        void addString(const char* text, float x, float y, float z, float sx, float sy, 
                glm::vec3 text_color, glm::mat4 transform);

        // Helper function mimicing old drawstring() call
        void addStrings(std::vector<std::string> & text_lines, 
                glm::vec3 screen_coordinates, 
                glm::vec3 text_color,
                glm::vec2 max_dimension);
        void addStrings(std::string text,
                std::vector<glm::vec3> screen_coordinates,
                glm::vec3 text_color);

        void Draw(glm::mat4 model_view, glm::mat4 projection);

    protected:
        ArgParser* m_args;

    private:
        void partitionString( std::vector<std::string> & split_strings, std::string original_string
                , std::vector<double> distances );
        virtual void BindUniforms(glm::mat4 model_view, glm::mat4 projection);


        FT_Library m_ft;
        std::vector<Font*> m_fonts;

        GLuint m_text_vao;
        GLuint m_text_vbo[3];

        std::vector<glm::vec3> m_text_position;
        std::vector<glm::vec2> m_text_uv;
        std::vector<glm::vec4> m_text_color;
};

#endif
