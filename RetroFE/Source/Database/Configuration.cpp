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
#include "Configuration.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include <algorithm>
#include <locale>
#include <fstream>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

std::string Configuration::AbsolutePath;

Configuration::Configuration()
    : Verbose(false)
{
}

Configuration::~Configuration()
{
}

void Configuration::Initialize()
{
    const char *environment = std::getenv("RETROFE_PATH");
    std::string environmentStr;
    if (environment != NULL)
    {
        environmentStr = environment;
        Configuration::SetAbsolutePath(environment);
    }
    else
    {
#ifdef WIN32
        HMODULE hModule = GetModuleHandle(NULL);
        CHAR exe[MAX_PATH];
        GetModuleFileName(hModule, exe, MAX_PATH);
        std::string sPath(exe);
        sPath = Utils::GetDirectory(sPath);
        sPath = Utils::GetParentDirectory(sPath);
#else
        char exepath[1024];
        sprintf(exepath, "/proc/%d/exe", getpid());
        readlink(exepath, exepath, sizeof(exepath));
        std::string sPath(exepath);
        sPath = Utils::GetDirectory(sPath);
#endif


        Configuration::SetAbsolutePath(sPath);
    }
}

void Configuration::SetCurrentCollection(std::string collection)
{
    CurrentCollection = collection;
}

std::string Configuration::GetCurrentCollection()
{
    return CurrentCollection;
}

bool Configuration::Import(std::string keyPrefix, std::string file)
{
    bool retVal = true;
    int lineCount = 0;
    std::string line;

    Logger::Write(Logger::ZONE_INFO, "Configuration", "Importing " + file);

    std::ifstream ifs(file.c_str());

    if (!ifs.is_open())
    {
        Logger::Write(Logger::ZONE_ERROR, "Configuration", "Could not open " + file);

        return false;
    }

    while (std::getline (ifs, line))
    {
        lineCount++;
        retVal = retVal && ParseLine(keyPrefix, line, lineCount);
    }

    ifs.close();

    return retVal;
}

bool Configuration::ParseLine(std::string keyPrefix, std::string line, int lineCount)
{
    bool retVal = false;
    std::string key;
    std::string value;
    size_t position;
    std::string delimiter = "=";

    // strip out any comments
    if((position = line.find("#")) != std::string::npos)
    {
        line = line.substr(0, position);
    }
    // unix only wants \n. Windows uses \r\n. Strip off the \r for unix.
    line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );
    if(line.empty() || (line.find_first_not_of(" \t\r") == std::string::npos))
    {
        retVal = true;
    }
    // all configuration fields must have an assignment operator
    else if((position = line.find(delimiter)) != std::string::npos)
    {
        if(keyPrefix.size() != 0)
        {
            keyPrefix += ".";
        }

        key = keyPrefix + line.substr(0, position);

        key = TrimEnds(key);


        value = line.substr(position + delimiter.length(), line.length());
        value = TrimEnds(value);

        Properties.insert(PropertiesPair(key, value));

        std::stringstream ss;
        ss << "Dump: "  << "\"" << key << "\" = \"" << value << "\"";


        Logger::Write(Logger::ZONE_INFO, "Configuration", ss.str());
        retVal = true;
    }
    else
    {
        std::stringstream ss;
        ss << "Missing an assignment operator (=) on line " << lineCount;
        Logger::Write(Logger::ZONE_ERROR, "Configuration", ss.str());
    }

    return retVal;
}

std::string Configuration::TrimEnds(std::string str)
{
    // strip off any initial tabs or spaces
    size_t trimStart = str.find_first_not_of(" \t");

    if(trimStart != std::string::npos)
    {
        size_t trimEnd = str.find_last_not_of(" \t");

        str = str.substr(trimStart, trimEnd - trimStart + 1);
    }

    return str;
}

bool Configuration::GetProperty(std::string key, std::string &value)
{
    bool retVal = false;

    if(Properties.find(key) != Properties.end())
    {
        value = Properties[key];

        retVal = true;
    }
    else if(Verbose)
    {
        Logger::Write(Logger::ZONE_DEBUG, "Configuration", "Missing property " + key);
    }

    value = Translate(value);

    return retVal;
}

bool Configuration::GetProperty(std::string key, int &value)
{
    std::string strValue;

    bool retVal = GetProperty(key, strValue);

    if(retVal)
    {
        std::stringstream ss;
        ss << strValue;
        ss >> value;
    }

    return retVal;
}

bool Configuration::GetProperty(std::string key, bool &value)
{
    std::string strValue;

    bool retVal = GetProperty(key, strValue);

    if(retVal)
    {
        if(!strValue.compare("yes") || !strValue.compare("true"))
        {
            value = true;
        }
        else
        {
            value = false;
        }
    }

    return retVal;
}

void Configuration::SetProperty(std::string key, std::string value)
{
    Properties[key] = value;
}

bool Configuration::PropertyExists(std::string key)
{
    return (Properties.find(key) != Properties.end());
}

bool Configuration::PropertyPrefixExists(std::string key)
{
    PropertiesType::iterator it;

    for(it = Properties.begin(); it != Properties.end(); ++it)
    {
        std::string search = key + ".";
        if(it->first.compare(0, search.length(), search) == 0)
        {
            return true;
        }
    }

    return false;
}

void Configuration::GetChildKeyCrumbs(std::string parent, std::vector<std::string> &children)
{
    PropertiesType::iterator it;

    for(it = Properties.begin(); it != Properties.end(); ++it)
    {
        std::string search = parent + ".";
        if(it->first.compare(0, search.length(), search) == 0)
        {
            std::string crumb = Utils::Replace(it->first, search, "");

            std::size_t end = crumb.find_first_of(".");

            if(end != std::string::npos)
            {
                crumb = crumb.substr(0, end);
            }

            if(std::find(children.begin(), children.end(), crumb) == children.end())
            {
                children.push_back(crumb);
            }
        }
    }
}

std::string Configuration::ConvertToAbsolutePath(std::string prefix, std::string path)
{
    char first = ' ';
    char second = ' ';

    if(path.length() >= 0)
    {
        first = path.c_str()[0];
    }
    if(path.length() >= 1)
    {
        second = path.c_str()[1];
    }

    // check to see if it is already an absolute path
    if((first != '/') &&
            (first != '\\') &&
            //(first != '.') &&
            (second != ':'))
    {
        path = prefix + "/" + path;
    }

    return path;
}

bool Configuration::GetPropertyAbsolutePath(std::string key, std::string &value)
{
    bool retVal = GetProperty(key, value);

    if(retVal)
    {
        value = ConvertToAbsolutePath(GetAbsolutePath(), value);
    }

    return retVal;
}

void Configuration::GetMediaPropertyAbsolutePath(std::string collectionName, std::string mediaType, std::string &value)
{
    std::string key = "media." + collectionName + "." + mediaType;

    if(!GetPropertyAbsolutePath(key, value))
    {
        std::string baseMediaPath;
        if(!GetPropertyAbsolutePath("baseMediaPath", baseMediaPath))
        {
            baseMediaPath = "Media";
        }

        value = baseMediaPath + "/" + collectionName + "/" + Utils::UppercaseFirst(Utils::ToLower(mediaType));
    }
}


void Configuration::SetAbsolutePath(std::string absolutePath)
{
    AbsolutePath = absolutePath;
}

std::string Configuration::GetAbsolutePath()
{
    return AbsolutePath;
}

bool Configuration::IsVerbose() const
{
    return Verbose;
}

void Configuration::SetVerbose(bool verbose)
{
    this->Verbose = verbose;
}

std::string Configuration::Translate(std::string str)
{
    std::string translated;
    std::size_t startIndex = 0;

    while(str.find("%") != std::string::npos) 
    {
        std::size_t startIndex = str.find("%");
        std::string var = str.substr(startIndex + 1);

        // copy everything before the first %
        translated += str.substr(0, startIndex);

        str = var; // discard the old unprocessed data up until the first %

        std::size_t endIndex = var.find("%");
        var = var.substr(0, endIndex);
        str = str.substr(endIndex + 1);

        std::string result;

        if(var == "collectionName") 
        {
            result = GetCurrentCollection();
        }
        else
        {
            GetProperty(var, result);
        }

        translated += result;
    }

    //copy the remaining string
    translated += str;
    return translated;
}

