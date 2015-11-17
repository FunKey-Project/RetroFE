#pragma once
class ComponentData
{
public:
    ComponentData()
    : width(0)
    , height(0)
    , x(0)
    , y(0)
    , alpha(1)
    , duration(0)
    , layer(0)
{
    }

    int width;
    int height;
    int x; 
    int y;
    float rotate;
    float alpha; 
    float duration;
    int layer;
};