/* This file is part of RetroFE.
 *
 * RetroFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RetroFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RetroFE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Utils.h"
#include "../Database/Configuration.h"
#include "Log.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <locale>
#include <list>


Utils::Utils()
{
}

Utils::~Utils()
{
}

std::string Utils::ToLower(std::string str)
{
    for(unsigned int i=0; i < str.length(); ++i)
    {
        std::locale loc;
        str[i] = std::tolower(str[i], loc);
    }

    return str;
}

std::string Utils::UppercaseFirst(std::string str)
{
    if(str.length() > 0)
    {
        std::locale loc;
        str[0] = std::toupper(str[0], loc);
    }

    return str;
}
std::string Utils::FilterComments(std:;string line)
{
    size_t position;

    // strip out any comments
    if((position = line.find("#")) != std::string::npos)
    {
        line = line.substr(0, position);
    }
    // unix only wants \n. Windows uses \r\n. Strip off the \r for unix.
    line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );
    
    return line;
}

std::string Utils::CombinePath(std::list<std::string> &paths)
{
    std::list<std::string>::iterator it = paths.begin();
    std::string path;

    if(it != paths.end())
    {
        path += *it;
        it++;
    }


    while(it != paths.end())
    {
        path += Utils::PathSeparator;
        path += *it;
        it++;
    }

    return path;
}

std::string Utils::CombinePath(std::string path1, std::string path2)
{
    std::list<std::string> paths;
    paths.push_back(path1);
    paths.push_back(path2);
    return CombinePath(paths);
}

std::string Utils::CombinePath(std::string path1, std::string path2, std::string path3)
{
    std::list<std::string> paths;
    paths.push_back(path1);
    paths.push_back(path2);
    paths.push_back(path3);
    return CombinePath(paths);
}

std::string Utils::CombinePath(std::string path1, std::string path2, std::string path3, std::string path4)
{
    std::list<std::string> paths;
    paths.push_back(path1);
    paths.push_back(path2);
    paths.push_back(path3);
    paths.push_back(path4);
    return CombinePath(paths);
}
std::string Utils::CombinePath(std::string path1, std::string path2, std::string path3, std::string path4, std::string path5)
{
    std::list<std::string> paths;
    paths.push_back(path1);
    paths.push_back(path2);
    paths.push_back(path3);
    paths.push_back(path4);
    paths.push_back(path5);
    return CombinePath(paths);
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

void Utils::ReplaceSlashesWithUnderscores(std::string &content)
{
    std::replace(content.begin(), content.end(), '\\', '_');
    std::replace(content.begin(), content.end(), '/', '_');
}


std::string Utils::GetDirectory(std::string filePath)
{

    std::string directory = filePath;

    const size_t last_slash_idx = filePath.rfind(PathSeparator);
    if (std::string::npos != last_slash_idx)
    {
        directory = filePath.substr(0, last_slash_idx);
    }

    return directory;
}

std::string Utils::GetParentDirectory(std::string directory)
{
    size_t last_slash_idx = directory.find_last_of(PathSeparator);
    if(directory.length() - 1 == last_slash_idx)
    {
        directory = directory.erase(last_slash_idx, directory.length()-1);
        last_slash_idx = directory.find_last_of(PathSeparator);
    }

    if (std::string::npos != last_slash_idx)
    {
        directory = directory.erase(last_slash_idx, directory.length());
    }

    return directory;
}


std::string Utils::GetFileName(std::string filePath)
{

    std::string filename = filePath;

    const size_t last_slash_idx = filePath.rfind(PathSeparator);
    if (std::string::npos != last_slash_idx)
    {
        filename = filePath.erase(0, last_slash_idx+1);
    }

    return filename;
}

