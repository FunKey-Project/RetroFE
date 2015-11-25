#pragma once
#include "../Component/Component.h"
#include "../Component/ComponentData.h"
#include "TweenTypes.h"
#include <string>

class Animation {
public:
    Component *component;
    ComponentData start;
    ComponentData end;
    float elapsedTime; 
    float duration;
    TweenAlgorithm algorithm;
};