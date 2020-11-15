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
#pragma once

#include <string>
#include <map>
#include <vector>

class Configuration
{
public:
    Configuration();
    virtual ~Configuration();
    static void initialize();
    static std::string convertToAbsolutePath(std::string prefix, std::string path);
    static std::string trimEnds(std::string str);
    // gets the global configuration
    bool import(std::string keyPrefix, std::string file);
    bool import(std::string collection, std::string keyPrefix, std::string file, bool mustExist = true);
    bool importLayouts(std::string folder, std::string file, bool mustExist = true);
    bool importCurrentLayout(std::string folder, std::string file, bool mustExist = true);
    bool exportCurrentLayout(std::string layoutFilePath, std::string layoutName);
    bool getProperty(std::string key, std::string &value);
    bool getProperty(std::string key, int &value);
    bool getProperty(std::string key, bool &value);
    void childKeyCrumbs(std::string parent, std::vector<std::string> &children);
    void setProperty(std::string key, std::string value);
    bool propertyExists(std::string key);
    bool propertyPrefixExists(std::string key);
    bool getPropertyAbsolutePath(std::string key, std::string &value);
    void getMediaPropertyAbsolutePath(std::string collectionName, std::string mediaType, std::string &value);
    void getMediaPropertyAbsolutePath(std::string collectionName, std::string mediaType, bool system, std::string &value);
    void getCollectionAbsolutePath(std::string collectionName, std::string &value);
    static std::string 				absolutePath;
    std::vector<std::string> 		layouts_;
    int							 	currentLayoutIdx_;

private:
    bool getRawProperty(std::string key, std::string &value);
    bool parseLine(std::string collection, std::string keyPrefix, std::string line, int lineCount);
    typedef std::map<std::string, std::string> PropertiesType;
    typedef std::pair<std::string, std::string> PropertiesPair;

    PropertiesType properties_;

};
