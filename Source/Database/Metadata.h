#pragma once

#include <string>

class Metadata
{
public:
    virtual ~Metadata() {}
    virtual bool Import(std::string file, std::string collectionName) = 0;
};
