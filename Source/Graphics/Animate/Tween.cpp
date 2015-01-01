/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Tween.h"
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

std::map<std::string, TweenAlgorithm> Tween::TweenTypeMap;
std::map<std::string, TweenProperty> Tween::TweenPropertyMap;

Tween::Tween(TweenProperty property, TweenAlgorithm type, double start, double end, double duration)
    : Property(property)
    , Type(type)
    , Start(start)
    , End(end)
    , Duration(duration)
{
}

TweenProperty Tween::GetProperty() const
{
    return Property;
}


bool Tween::GetTweenProperty(std::string name, TweenProperty &property)
{
    bool retVal = false;

    if(TweenPropertyMap.size() == 0)
    {
        TweenPropertyMap["x"] = TWEEN_PROPERTY_X;
        TweenPropertyMap["y"] = TWEEN_PROPERTY_Y;
        TweenPropertyMap["angle"] = TWEEN_PROPERTY_ANGLE;
        TweenPropertyMap["transparency"] = TWEEN_PROPERTY_TRANSPARENCY;
        TweenPropertyMap["width"] = TWEEN_PROPERTY_WIDTH;
        TweenPropertyMap["height"] = TWEEN_PROPERTY_HEIGHT;
        TweenPropertyMap["xorigin"] = TWEEN_PROPERTY_X_ORIGIN;
        TweenPropertyMap["yorigin"] = TWEEN_PROPERTY_Y_ORIGIN;
        TweenPropertyMap["xoffset"] = TWEEN_PROPERTY_X_OFFSET;
        TweenPropertyMap["yoffset"] = TWEEN_PROPERTY_Y_OFFSET;
        TweenPropertyMap["fontSize"] = TWEEN_PROPERTY_FONT_SIZE;
    }

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if(TweenPropertyMap.find(name) != TweenPropertyMap.end())
    {
        property = TweenPropertyMap[name];
        retVal = true;
    }

    return retVal;
}


TweenAlgorithm Tween::GetTweenType(std::string name)
{
    if(TweenTypeMap.size() == 0)
    {
        TweenTypeMap["easeinquadratic"] = EASE_IN_QUADRATIC;
        TweenTypeMap["easeoutquadratic"] = EASE_OUT_QUADRATIC;
        TweenTypeMap["easeinoutquadratic"] = EASE_INOUT_QUADRATIC;
        TweenTypeMap["easeincubic"] = EASE_IN_CUBIC;
        TweenTypeMap["easeoutcubic"] = EASE_OUT_CUBIC;
        TweenTypeMap["easeinoutcubic"] = EASE_INOUT_CUBIC;
        TweenTypeMap["easeinquartic"] = EASE_IN_QUARTIC;
        TweenTypeMap["easeoutquartic"] = EASE_OUT_QUARTIC;
        TweenTypeMap["easeinoutquartic"] = EASE_INOUT_QUARTIC;
        TweenTypeMap["easeinquintic"] = EASE_IN_QUINTIC;
        TweenTypeMap["easeoutquintic"] = EASE_OUT_QUINTIC;
        TweenTypeMap["easeinoutquintic"] = EASE_INOUT_QUINTIC;
        TweenTypeMap["easeinsine"] = EASE_IN_SINE;
        TweenTypeMap["easeoutsine"] = EASE_OUT_SINE;
        TweenTypeMap["easeinoutsine"] = EASE_INOUT_SINE;
        TweenTypeMap["easeinexponential"] = EASE_IN_EXPONENTIAL;
        TweenTypeMap["easeoutexponential"] = EASE_OUT_EXPONENTIAL;
        TweenTypeMap["easeinoutexponential"] = EASE_INOUT_EXPONENTIAL;
        TweenTypeMap["easeincircular"] = EASE_IN_CIRCULAR;
        TweenTypeMap["easeoutcircular"] = EASE_OUT_CIRCULAR;
        TweenTypeMap["easeinoutcircular"] = EASE_INOUT_CIRCULAR;
        TweenTypeMap["linear"] = LINEAR;
    }

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if(TweenTypeMap.find(name) != TweenTypeMap.end())
    {
        return TweenTypeMap[name];
    }
    else
    {
        return TweenTypeMap["linear"];
    }
}


float Tween::Animate(double elapsedTime)
{
    return AnimateSingle(Type, Start, End, Duration, elapsedTime);
}

//todo: SDL likes floats, consider having casting being performed elsewhere
float Tween::AnimateSingle(TweenAlgorithm type, double start, double end, double duration, double elapsedTime)
{
    double a = start;
    double b = end - start;
    double result = 0;

    switch(type)
    {
    case EASE_IN_QUADRATIC:
        result = EaseInQuadratic(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_QUADRATIC:
        result = EaseOutQuadratic(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_QUADRATIC:
        result = EaseInOutQuadratic(elapsedTime, duration, a, b);
        break;

    case EASE_IN_CUBIC:
        result = EaseInCubic(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_CUBIC:
        result = EaseOutCubic(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_CUBIC:
        result = EaseInOutCubic(elapsedTime, duration, a, b);
        break;

    case EASE_IN_QUARTIC:
        result = EaseInQuartic(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_QUARTIC:
        result = EaseOutQuartic(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_QUARTIC:
        result = EaseInOutQuartic(elapsedTime, duration, a, b);
        break;

    case EASE_IN_QUINTIC:
        result = EaseInQuintic(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_QUINTIC:
        result = EaseOutQuintic(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_QUINTIC:
        result = EaseInOutQuintic(elapsedTime, duration, a, b);
        break;

    case EASE_IN_SINE:
        result = EaseInSine(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_SINE:
        result = EaseOutSine(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_SINE:
        result = EaseInOutSine(elapsedTime, duration, a, b);
        break;

    case EASE_IN_EXPONENTIAL:
        result = EaseInExponential(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_EXPONENTIAL:
        result = EaseOutExponential(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_EXPONENTIAL:
        result = EaseInOutExponential(elapsedTime, duration, a, b);
        break;

    case EASE_IN_CIRCULAR:
        result = EaseInCircular(elapsedTime, duration, a, b);
        break;

    case EASE_OUT_CIRCULAR:
        result = EaseOutCircular(elapsedTime, duration, a, b);
        break;

    case EASE_INOUT_CIRCULAR:
        result = EaseInOutCircular(elapsedTime, duration, a, b);
        break;

    case LINEAR:
    default:
        result = Linear(elapsedTime, duration, a, b);
        break;
    }

    return static_cast<float>(result);

}

double Tween::Linear(double t, double d, double b, double c)
{
    if(d == 0) return b;
    return c*t/d + b;
};

double Tween::EaseInQuadratic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return c*t*t + b;
};

double Tween::EaseOutQuadratic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return -c * t*(t-2) + b;
};

double Tween::EaseInOutQuadratic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d/2;
    if (t < 1) return c/2*t*t + b;
    t--;
    return -c/2 * (t*(t-2) - 1) + b;
};

double Tween::EaseInCubic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return c*t*t*t + b;
};

double Tween::EaseOutCubic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    t--;
    return c*(t*t*t + 1) + b;
};

double Tween::EaseInOutCubic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d/2;
    if (t < 1) return c/2*t*t*t + b;
    t -= 2;
    return c/2*(t*t*t + 2) + b;
};

double Tween::EaseInQuartic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return c*t*t*t*t + b;
};

double Tween::EaseOutQuartic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    t--;
    return -c * (t*t*t*t - 1) + b;
};

double Tween::EaseInOutQuartic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d/2;
    if (t < 1) return c/2*t*t*t*t + b;
    t -= 2;
    return -c/2 * (t*t*t*t - 2) + b;
};

double Tween::EaseInQuintic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    return c*t*t*t*t*t + b;
};


double Tween::EaseOutQuintic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d;
    t--;
    return c*(t*t*t*t*t + 1) + b;
};

double Tween::EaseInOutQuintic(double t, double d, double b, double c)
{
    if(d == 0) return b;
    t /= d/2;
    if (t < 1) return c/2*t*t*t*t*t + b;
    t -= 2;
    return c/2*(t*t*t*t*t + 2) + b;
};

double Tween::EaseInSine(double t, double d, double b, double c)
{
    return -c * cos(t/d * (M_PI/2)) + c + b;
};

double Tween::EaseOutSine(double t, double d, double b, double c)
{
    return c * sin(t/d * (M_PI/2)) + b;
};

double Tween::EaseInOutSine(double t, double d, double b, double c)
{
    return -c/2 * (cos( M_PI*t/d) - 1) + b;
};

double Tween::EaseInExponential(double t, double d, double b, double c)
{
    return c * pow( 2, 10 * (t/d - 1) ) + b;
};

double Tween::EaseOutExponential(double t, double d, double b, double c)
{
    return c * ( - pow( 2, -10 * t/d ) + 1 ) + b;
};

double Tween::EaseInOutExponential(double t, double d, double b, double c)
{
    t /= d/2;
    if (t < 1) return c/2 * pow( 2, 10 * (t - 1) ) + b;
    t--;
    return c/2 * ( -1* pow( 2, -10 * t) + 2 ) + b;
};

double Tween::EaseInCircular(double t, double d, double b, double c)
{
    t /= d;
    return -c * (sqrt(1 - t*t) - 1) + b;
};


double Tween::EaseOutCircular(double t, double d, double b, double c)
{
    t /= d;
    t--;
    return c * sqrt(1 - t*t) + b;
};

double Tween::EaseInOutCircular(double t, double d, double b, double c)
{
    t /= d/2;
    if (t < 1) return -c/2 * (sqrt(1 - t*t) - 1) + b;
    t -= 2;
    return c/2 * (sqrt(1 - t*t) + 1) + b;
}
;

//todo: sdl requires floats, should the casting be done at this layer?
float Tween::GetDuration() const
{
    return static_cast<float>(Duration);
}
