#pragma once

#include "InputHandler.h"

class JoyHatHandler : public InputHandler
{
public:
    JoyHatHandler(Uint8 direction);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();

private:
    Uint8 direction_;
    bool pressed_;
};

