#pragma once 

#include "Image.h"
#include "Component.h"
#include <map>

class ComponentFactory
{
public:
  Image *createImage();
  void deleteComponent(Component *c);
  std::map<Component *, Component *> components;
};