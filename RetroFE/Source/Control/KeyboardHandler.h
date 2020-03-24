#pragma once

#include "InputHandler.h"

class KeyboardHandler : public InputHandler
{
public:
    KeyboardHandler(SDLKey scancode);
    bool update(SDL_Event &e);
    bool pressed();
    void reset();

private:
    SDLKey scancode_;
    bool pressed_;
};

