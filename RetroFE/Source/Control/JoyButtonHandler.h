#pragma once

#include "InputHandler.h"

class JoyButtonHandler : public InputHandler
{
public:
    JoyButtonHandler(Uint8 button);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();

private:
    Uint8 button_;
    bool pressed_;
};

