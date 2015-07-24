#include "KeyboardHandler.h"

KeyboardHandler::KeyboardHandler(SDL_Scancode s)
: scancode_(s)
, pressed_(false)
{
}

void KeyboardHandler::reset()
{
    pressed_= false;
}

bool KeyboardHandler::update(SDL_Event &e)
{
    if(e.key.keysym.scancode == scancode_) 
    {
        if(e.type == SDL_KEYUP) pressed_ = false;
        if(e.type == SDL_KEYDOWN) pressed_ = true;
        return true;
    }

    return false;
}

bool KeyboardHandler::pressed()
{
    return pressed_;
}

