#include "ComponentFactory.h"

Image *ComponentFactory::createImage()
{
  Image *i = new Image();
  components[i] = i;

  return i;
}

void ComponentFactory::deleteComponent(Component *c)
{
  std::map<Component *, Component *>::iterator it = components.find(c);

  if(it != components.end()) {
    delete it->first;
    components.erase(it);
  }
}
