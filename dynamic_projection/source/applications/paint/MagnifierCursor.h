#ifndef MAGNIFIER_CURSOR_H_
#define MAGNIFIER_CURSOR_H_

#include <vector>
#include <map>
#include "gl_includes.h"

// Static wrapper around mouse data

class MagnifierCursor {
    public:
        MagnifierCursor();

        glm::vec2 m_position;
        int m_status;
        int m_strength;
};

#endif
