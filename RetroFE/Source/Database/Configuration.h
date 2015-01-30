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
    static void Initialize();
    static void SetAbsolutePath(std::string absolutePath);
    static std::string GetAbsolutePath();
    static std::string ConvertToAbsolutePath(std::string prefix, std::string path);
    void SetStatus(std::string status);
    std::string GetStatus();
    // gets the global configuration
    bool Import(std::string keyPrefix, std::string file);
    void SetCurrentCollection(std::string collection);
    std::string GetCurrentCollection();
    bool GetProperty(std::string key, std::string &value);
    bool GetProperty(std::string key, int &value);
    bool GetProperty(std::string key, bool &value);
    void GetChildKeyCrumbs(std::string parent, std::vector<std::string> &children);
    void SetProperty(std::string key, std::string value);
    bool PropertyExists(std::string key);
    bool PropertyPrefixExists(std::string key);
    bool GetPropertyAbsolutePath(std::string key, std::string &value);
    void GetMediaPropertyAbsolutePath(std::string collectionName, std::string mediaType, std::string &value);
    void GetCollectionAbsolutePath(std::string collectionName, std::string &value);
    bool IsVerbose() const;
    void SetVerbose(bool verbose);

private:
    bool GetRawProperty(std::string key, std::string &value);
    bool ParseLine(std::string keyPrefix, std::string line, int lineCount);
    std::string TrimEnds(std::string str);
    typedef std::map<std::string, std::string> PropertiesType;
    typedef std::pair<std::string, std::string> PropertiesPair;
    bool Verbose;

    static std::string AbsolutePath;
    std::string CurrentCollection;
    PropertiesType Properties;
    std::string Status;

};
