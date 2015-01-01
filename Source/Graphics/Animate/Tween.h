/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "TweenTypes.h"
#include <string>
#include <map>

class ViewInfo;

class Tween
{
public:

    Tween(TweenProperty name, TweenAlgorithm type, double start, double end, double duration);
    float Animate(double elapsedTime);
    static float AnimateSingle(TweenAlgorithm type, double start, double end, double duration, double elapsedTime);
    static TweenAlgorithm GetTweenType(std::string name);
    static bool GetTweenProperty(std::string name, TweenProperty &property);
    TweenProperty GetProperty() const;
    float GetDuration() const;

private:
    static double EaseInQuadratic(double elapsedTime, double duration, double b, double c);
    static double EaseOutQuadratic(double elapsedTime, double duration, double b, double c);
    static double EaseInOutQuadratic(double elapsedTime, double duration, double b, double c);
    static double EaseInCubic(double elapsedTime, double duration, double b, double c);
    static double EaseOutCubic(double elapsedTime, double duration, double b, double c);
    static double EaseInOutCubic(double elapsedTime, double duration, double b, double c);
    static double EaseInQuartic(double elapsedTime, double duration, double b, double c);
    static double EaseOutQuartic(double elapsedTime, double duration, double b, double c);
    static double EaseInOutQuartic(double elapsedTime, double duration, double b, double c);
    static double EaseInQuintic(double elapsedTime, double duration, double b, double c);
    static double EaseOutQuintic(double elapsedTime, double duration, double b, double c);
    static double EaseInOutQuintic(double elapsedTime, double duration, double b, double c);
    static double EaseInSine(double elapsedTime, double duration, double b, double c);
    static double EaseOutSine(double elapsedTime, double duration, double b, double c);
    static double EaseInOutSine(double elapsedTime, double duration, double b, double c);
    static double EaseInExponential(double elapsedTime, double duration, double b, double c);
    static double EaseOutExponential(double elapsedTime, double duration, double b, double c);
    static double EaseInOutExponential(double elapsedTime, double duration, double b, double c);
    static double EaseInCircular(double elapsedTime, double duration, double b, double c);
    static double EaseOutCircular(double elapsedTime, double duration, double b, double c);
    static double EaseInOutCircular(double elapsedTime, double duration, double b, double c);
    static double Linear(double elapsedTime, double duration, double b, double c);

    static std::map<std::string, TweenAlgorithm> TweenTypeMap;
    static std::map<std::string, TweenProperty> TweenPropertyMap;
    TweenProperty Property;
    TweenAlgorithm Type;
    double Start;
    double End;
    double Duration;
};
