#include "JoyHatHandler.h"

JoyHatHandler::JoyHatHandler(Uint8 direction)
: direction_(direction)
, pressed_(false)
{
}

void JoyHatHandler::reset()
{
    pressed_= false;
}

bool JoyHatHandler::update(SDL_Event &e)
{
    if(e.type != SDL_JOYHATMOTION) return false;

    pressed_ = (e.jhat.value == direction_);
    return true;
}

bool JoyHatHandler::pressed()
{
    return pressed_;
}

