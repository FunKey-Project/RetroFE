/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Utils.h"
#include "../Database/Configuration.h"
#include "Log.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <dirent.h>

Utils::Utils()
{
}

Utils::~Utils()
{
}


bool Utils::FindMatchingFile(std::string prefix, std::vector<std::string> &extensions, std::string &file)
{
    for(unsigned int i = 0; i < extensions.size(); ++i)
    {
        std::string temp = prefix + "." + extensions[i];
        temp = Configuration::ConvertToAbsolutePath(Configuration::GetAbsolutePath(), temp);

        std::ifstream f(temp.c_str());

        if (f.good())
        {
            file = temp;
            return true;
        }
    }

    return false;
}


std::string Utils::Replace(
    std::string subject,
    const std::string& search,
    const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}


float Utils::ConvertFloat(std::string content)
{
    float retVal = 0;
    std::stringstream ss;
    ss << content;
    ss >> retVal;

    return retVal;
}

int Utils::ConvertInt(std::string content)
{
    int retVal = 0;
    std::stringstream ss;
    ss << content;
    ss >> retVal;

    return retVal;
}

void Utils::NormalizeBackSlashes(std::string& content)
{
    std::replace(content.begin(), content.end(), '\\', '/');
}

std::string Utils::GetDirectory(std::string filePath)
{

    NormalizeBackSlashes(filePath);
    std::string directory = filePath;

    const size_t last_slash_idx = filePath.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
        directory = filePath.substr(0, last_slash_idx);
    }

    return directory;
}

std::string Utils::GetParentDirectory(std::string directory)
{

    NormalizeBackSlashes(directory);

    size_t last_slash_idx = directory.find_last_of('/');
    if(directory.length() - 1 == last_slash_idx)
    {
        directory = directory.erase(last_slash_idx, directory.length()-1);
        last_slash_idx = directory.find_last_of('/');
    }

    if (std::string::npos != last_slash_idx)
    {
        directory = directory.erase(last_slash_idx, directory.length());
    }

    return directory;
}


std::string Utils::GetFileName(std::string filePath)
{

    NormalizeBackSlashes(filePath);
    std::string filename = filePath;

    const size_t last_slash_idx = filePath.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
        filename = filePath.erase(0, last_slash_idx+1);
    }

    return filename;
}

