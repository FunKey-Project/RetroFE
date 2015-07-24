#include "JoyAxisHandler.h"

JoyAxisHandler::JoyAxisHandler(Uint8 axis, Sint16 min, Sint16 max)
: axis_(axis)
, min_(min)
, max_(max)
, pressed_(false)
{
}

void JoyAxisHandler::reset()
{
    pressed_= false;
}

bool JoyAxisHandler::update(SDL_Event &e)
{
    if(e.type != SDL_JOYAXISMOTION || e.jaxis.axis != axis_) return false;
    pressed_ = (min_ <= e.jaxis.value && e.jaxis.value <= max_);

    return true;
}

bool JoyAxisHandler::pressed()
{
    return pressed_;
}

