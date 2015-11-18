#pragma once

#define COMPONENT_DATA_X_MASK  0x1
#define COMPONENT_DATA_Y_MASK 0x2
#define COMPONENT_DATA_WIDTH_MASK 0x4
#define COMPONENT_DATA_HEIGHT_MASK 0x8
#define COMPONENT_DATA_ROTATE_MASK 0x10
#define COMPONENT_DATA_ALPHA_MASK 0x10
#define COMPONENT_DATA_DURATION_MASK 0x20
#define COMPONENT_DATA_LAYER_MASK 0x20
#define COMPONENT_DATA_ALL_MASK 0xFF

class ComponentData
{
public:
    ComponentData()
    : x(0)
    , y(0)
    , width(0)
    , height(0)
    , rotate(0)
    , alpha(1)
    , duration(0)
    , layer(0)
    , mask(COMPONENT_DATA_ALL_MASK)
{
    }
public:
    int x; 
    int y;
    int width;
    int height;
    float rotate;
    float alpha; 
    float duration; 
    int layer;
    void setMask(unsigned char flag) { mask |= flag; }
    void clearMask(unsigned char flag) { mask &= ~flag; }
    bool isMaskSet(unsigned char flag) { return ((mask & flag) != 0); }

private:
    unsigned char mask;
};
