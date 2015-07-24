#pragma once

#include "InputHandler.h"

class JoyAxisHandler : public InputHandler
{
public:
    JoyAxisHandler(Uint8 axis, Sint16 min, Sint16 max);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();

private:
    Uint8 axis_;
    Sint16 min_;
    Sint16 max_;
        
    bool pressed_;
};

