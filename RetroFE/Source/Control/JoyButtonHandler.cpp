#include "JoyButtonHandler.h"

JoyButtonHandler::JoyButtonHandler(Uint8 button)
: button_(button)
, pressed_(false)
{
}

void JoyButtonHandler::reset()
{
    pressed_= false;
}

bool JoyButtonHandler::update(SDL_Event &e)
{
    if(e.type != SDL_JOYBUTTONUP && e.type != SDL_JOYBUTTONDOWN) return false;

    if(e.jbutton.button == button_)
    {
        pressed_ = (e.type == SDL_JOYBUTTONDOWN) ? true : false;
        return true;
    }

    return false;
}

bool JoyButtonHandler::pressed()
{
    return pressed_;
}

